/**
 * @file inverted_index.cpp
 * @author Sean Massung
 */
#include <iostream>
#include "index/inverted_index.h"
#include "index/chunk.h"

namespace meta {
namespace index {

inverted_index::inverted_index(const cpptoml::toml_group & config):
    base{config, *cpptoml::get_as<std::string>(config, "inverted-index")}
{ /* nothing */ }

std::vector<postings_data<term_id, doc_id>> inverted_index::to_vector(
        std::unordered_map<term_id, postings_data<term_id, doc_id>> & pdata)
{
    std::vector<postings_data_type> vec;
    vec.reserve(pdata.size());
    for(auto & p: pdata)
        vec.push_back(p.second);
    pdata.clear();
    return vec;
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
        for(auto & sz: _doc_sizes)
            _total_corpus_terms += sz;
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

double inverted_index::avg_doc_length()
{
    return static_cast<double>(_total_corpus_terms) / _doc_sizes.size();
}

void inverted_index::chunk_handler::handle_doc(const corpus::document & doc) {
    for (const auto & f : doc.frequencies()) {
        postings_data_type pd{f.first};
        pd.increase_count(doc.id(), f.second);
        auto it = pdata_.find(f.first);
        if (it == pdata_.end())
            pdata_.emplace(f.first, pd);
        else
            it->second.merge_with(pd);
    }
}

std::vector<postings_data<term_id, doc_id>>
inverted_index::chunk_handler::chunk() {
    return to_vector(pdata_);
}

inverted_index::chunk_handler::~chunk_handler() {
    write_chunk();
}

}
}
