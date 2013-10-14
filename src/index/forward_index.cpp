/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <memory>
#include <unordered_set>
#include "index/forward_index.h"
#include "index/chunk.h"

using std::cerr;
using std::endl;

namespace meta {
namespace index {


forward_index::forward_index(const cpptoml::toml_group & config):
    disk_index{config, *cpptoml::get_as<std::string>(config, "forward-index")}
{ /* nothing */ }

uint32_t forward_index::tokenize_docs(
        const std::unique_ptr<corpus::corpus> & docs)
{
    std::atomic<uint32_t> chunk_num{0};
    const uint32_t chunk_size = 500;
    std::string progress = "Tokenizing ";
    std::mutex mutex;

    // allocate metadata
    _doc_id_mapping.resize(docs->size());
    _doc_sizes.resize(docs->size());
    _unique_terms.resize(docs->size());
    _labels.resize(docs->size());

    auto task = [&]() {
        std::vector<postings_data<doc_id, term_id>> pdata;
        pdata.reserve(chunk_size);
        while (true) {
            util::optional<corpus::document> doc;
            {
                std::lock_guard<std::mutex> lock{mutex};
                if (docs->has_next()) {
                    doc = docs->next();
                    common::show_progress(doc->id(), docs->size(), 20, progress);
                }
            }

            if (!doc) {
                if (!pdata.empty())
                    write_chunk(chunk_num.fetch_add(1), pdata);
                return;
            }

            _tokenizer->tokenize(*doc);

            _doc_id_mapping[doc->id()] = doc->path();
            _doc_sizes[doc->id()] = doc->length();

            postings_data<doc_id, term_id> pd{doc->id()};
            pd.set_counts(std::vector<std::pair<term_id, double>>{
                doc->frequencies().begin(), doc->frequencies().end()
            });
            pdata.push_back(pd);

            _unique_terms[doc->id()] = doc->frequencies().size();
            _labels[doc->id()] = doc->label();

            if (pdata.size() % chunk_size == 0)
                write_chunk(chunk_num.fetch_add(1), pdata);
        }
    };

    parallel::thread_pool pool;
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < pool.thread_ids().size(); ++i)
        futures.emplace_back(pool.submit_task(task));

    for (auto & fut : futures)
        fut.get();
    common::end_progress(progress);

    return chunk_num;
}

std::string forward_index::liblinear_data(doc_id d_id) const
{
    auto pdata = search_primary(d_id);

    // output the class label (starting with index 1)

    std::stringstream out;
    out << (label_id_from_doc(d_id) + 1);

    // output each term_id:count (starting with index 1)

    using term_pair = std::pair<term_id, double>;
    std::vector<term_pair> sorted;
    sorted.reserve(pdata->counts().size());
    for(auto & p: pdata->counts())
        sorted.emplace_back(std::piecewise_construct,
                            std::forward_as_tuple(p.first + 1),
                            std::forward_as_tuple(p.second));

    std::sort(sorted.begin(), sorted.end(),
        [](const term_pair & a, const term_pair & b) {
            return a.first < b.first;
        }
    );

    for(auto & freq: sorted)
        out << " " << freq.first << ":" << freq.second;

    out << "\n";
    return out.str();
}

}
}
