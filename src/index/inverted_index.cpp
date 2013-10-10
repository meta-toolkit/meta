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

uint32_t inverted_index::tokenize_docs(
        const std::unique_ptr<corpus::corpus> & docs)
{
    std::unordered_map<term_id, PostingsData> pdata;
    uint32_t chunk_num = 0;
    std::string progress = "Tokenizing ";
    while(docs->has_next())
    {
        corpus::document doc{docs->next()};
        common::show_progress(doc.id(), docs->size(), 20, progress);
        _tokenizer->tokenize(doc);
        _doc_id_mapping[doc.id()] = doc.path();
        _doc_sizes[doc.id()] = doc.length();
        _total_corpus_terms += doc.length();

        for(auto & f: doc.frequencies())
        {
            PostingsData pd{f.first};
            pd.increase_count(doc.id(), f.second);
            auto it = pdata.find(f.first);
            if(it == pdata.end())
                pdata.insert(std::make_pair(f.first, pd));
            else
                it->second.merge_with(pd);
        }

        // Save class label information
        _unique_terms[doc.id()] = doc.frequencies().size();
        if(doc.label() != class_label{""})
            _labels[doc.id()] = doc.label();

        // every k documents, write a chunk
        // TODO: make this based on memory usage instead
        if(doc.id() % 500 == 0)
        {
            std::vector<PostingsData> vec{to_vector(pdata)};
            write_chunk(chunk_num++, vec);
        }
    }
    common::end_progress(progress);

    if(!pdata.empty())
    {
        std::vector<PostingsData> vec{to_vector(pdata)};
        write_chunk(chunk_num++, vec);
    }

    return chunk_num;
}

std::vector<postings_data<term_id, doc_id>> inverted_index::to_vector(
        std::unordered_map<term_id, postings_data<term_id, doc_id>> & pdata)
{
    std::vector<PostingsData> vec;
    vec.reserve(pdata.size());
    for(auto & p: pdata)
        vec.push_back(p.second);
    pdata.clear();
    return vec;
}

uint64_t inverted_index::idf(term_id t_id) const
{
    auto pdata = search_primary(t_id);
    return pdata->inverse_frequency();
}

double inverted_index::term_freq(term_id t_id, doc_id d_id) const
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

double inverted_index::total_num_occurences(term_id t_id) const
{
    auto pdata = search_primary(t_id);

    double sum = 0;
    for(auto & c: pdata->counts())
        sum += c.second;

    return sum;
}

const std::vector<std::pair<doc_id, double>>
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
