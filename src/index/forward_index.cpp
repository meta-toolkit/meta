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
{
    set_label_ids();
}

void forward_index::set_label_ids()
{
    std::unordered_set<class_label> labels;
    for(auto & p: _labels)
        labels.insert(p.second);

    size_t i = 0;
    for(auto & lbl: labels)
        _label_ids.insert(lbl, i++);
}

uint32_t forward_index::tokenize_docs(std::vector<document> & docs)
{
    std::unordered_map<term_id, postings_data<doc_id, term_id>> pdata;
    uint32_t chunk_num = 0;
    uint64_t doc_num = 0;
    std::string progress = "Tokenizing ";
    for(auto & doc: docs)
    {
        common::show_progress(doc_num, docs.size(), 20, progress);
        _tokenizer->tokenize(doc);
        _doc_id_mapping[doc_num] = doc.path();
        _doc_sizes[doc_num] = doc.length();

        if(doc.label() != "")
            _labels[doc_num] = doc.label();

        postings_data<doc_id, term_id> pd{doc_num};
        pd.set_counts(doc.frequencies());

        // in the current scheme, we should never have to merge two postings
        // together in this step since each postings is a unique doc_id
        auto it = pdata.find(doc_num);
        if(it == pdata.end())
            pdata.insert(std::make_pair(doc_num, pd));
        else
            it->second.merge_with(pd);

        ++doc_num;

        // every k documents, write a chunk
        // TODO: make this based on memory usage instead
        if(doc_num % 100 == 0)
            write_chunk(chunk_num++, pdata);
    }
    common::end_progress(progress);
    
    if(!pdata.empty())
        write_chunk(chunk_num++, pdata);

    return chunk_num;
}

class_label forward_index::label(doc_id d_id) const
{
    return common::safe_at(_labels, d_id);
}

const std::unordered_map<term_id, uint64_t> forward_index::counts(doc_id d_id) const
{
    postings_data<doc_id, term_id> pdata = search_primary(d_id);
    return pdata.counts();
}

class_label forward_index::class_label_from_id(label_id l_id) const
{
    return _label_ids.get_key(l_id);
}

std::string forward_index::liblinear_data(doc_id d_id) const
{
    postings_data<doc_id, term_id> pdata = search_primary(d_id);

    // output the class label (starting with index 1)

    std::stringstream out;
    out << (_label_ids.get_value(_labels.at(d_id)) + 1);

    // output each term_id:count (starting with index 1)

    std::vector<std::pair<term_id, uint64_t>> sorted;
    sorted.reserve(pdata.counts().size());
    for(auto & p: pdata.counts())
        sorted.push_back(std::make_pair(p.first + 1, p.second));

    std::sort(sorted.begin(), sorted.end(),
        [](const std::pair<term_id, uint64_t> & a, const std::pair<term_id, uint64_t> & b) {
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
