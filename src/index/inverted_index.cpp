/**
 * @file inverted_index.cpp
 * @author Sean Massung
 */
#include <iostream>
#include "index/inverted_index.h"
#include "index/chunk.h"

using std::cerr;
using std::endl;

namespace meta {
namespace index {

inverted_index::inverted_index(const cpptoml::toml_group & config):
    disk_index{config, *cpptoml::get_as<std::string>(config, "inverted-index")},
    _avg_dl{-1.0},
    _total_corpus_terms{0}
{ /* nothing */ }

uint32_t inverted_index::tokenize_docs(std::vector<document> & docs)
{
    std::unordered_map<term_id, postings_data<term_id, doc_id>> pdata;
    uint32_t chunk_num = 0;
    doc_id doc_num{0};
    std::string progress = "Tokenizing ";
    for(auto & doc: docs)
    {
        common::show_progress(doc_num, docs.size(), 20, progress);
        _tokenizer->tokenize(doc);
        _doc_id_mapping[doc_num] = doc.path();
        _doc_sizes[doc_num] = doc.length();
        _total_corpus_terms += doc.length();

        for(auto & f: doc.frequencies())
        {
            postings_data<term_id, doc_id> pd{f.first};
            pd.increase_count(doc_num, f.second);
            auto it = pdata.find(f.first);
            if(it == pdata.end())
                pdata.insert(std::make_pair(f.first, pd));
            else
                it->second.merge_with(pd);
        }

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

uint64_t inverted_index::idf(term_id t_id) const
{
    auto pdata = search_primary(t_id);
    return pdata->inverse_frequency();
}

uint64_t inverted_index::term_freq(term_id t_id, doc_id d_id) const
{
    auto pdata = search_primary(t_id);
    return pdata->count(d_id);
}

uint64_t inverted_index::total_corpus_terms()
{
    if(_total_corpus_terms == 0)
    {
        for(auto & d: _doc_sizes)
            _total_corpus_terms += d.second;
    }

    return _total_corpus_terms;
}

uint64_t inverted_index::total_num_occurences(term_id t_id) const
{
    auto pdata = search_primary(t_id);

    uint64_t sum = 0;
    for(auto & c: pdata->counts())
        sum += c.second;

    return sum;
}

const std::unordered_map<doc_id, uint64_t>
inverted_index::counts(term_id t_id) const
{
    auto pdata = search_primary(t_id);
    return pdata->counts();
}

double inverted_index::avg_doc_length()
{
    if(_avg_dl == -1.0)
    {
        uint64_t sum = 0.0;
        for(auto & p: _doc_sizes)
            sum += p.second;
        _avg_dl = static_cast<double>(sum) / _doc_sizes.size();
    }

    return _avg_dl;
}

}
}
