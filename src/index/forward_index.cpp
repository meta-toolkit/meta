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
    std::unordered_map<doc_id, postings_data<doc_id, term_id>> pdata;
    uint32_t chunk_num = 0;
    std::string progress = "Tokenizing ";
    while(docs->has_next())
    {
        corpus::document doc{docs->next()};
        common::show_progress(doc.id(), docs->size(), 20, progress);
        _tokenizer->tokenize(doc);
        _doc_id_mapping[doc.id()] = doc.path();
        _doc_sizes[doc.id()] = doc.length();

        postings_data<doc_id, term_id> pd{doc.id()};
        pd.set_counts(std::vector<std::pair<term_id, double>>{
            doc.frequencies().begin(), doc.frequencies().end()
        });

        // in the current scheme, we should never have to merge two postings
        // together in this step since each postings is a unique doc_id
        auto it = pdata.find(doc.id());
        if(it == pdata.end())
            pdata.insert(std::make_pair(doc.id(), pd));
        else
            it->second.merge_with(pd);

        // Save class label information
        _unique_terms[doc.id()] = doc.frequencies().size();
        if(doc.label() != class_label{""})
            _labels[doc.id()] = doc.label();

        // every k documents, write a chunk
        // TODO: make this based on memory usage instead
        if(doc.id() % 500 == 0)
            write_chunk(chunk_num++, pdata);
    }
    common::end_progress(progress);
   
    if(!pdata.empty())
        write_chunk(chunk_num++, pdata);

    return chunk_num;
}

const std::vector<std::pair<term_id, double>>
forward_index::counts(doc_id d_id) const
{
    auto pdata = search_primary(d_id);
    return pdata->counts();
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
