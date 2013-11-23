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

double inverted_index::term_freq(term_id t_id, doc_id d_id) const
{
    auto pdata = search_primary(t_id);
    return pdata->count(d_id);
}

uint64_t inverted_index::total_corpus_terms()
{
    if(_total_corpus_terms == 0)
    {
        for(auto & sz: *_doc_sizes)
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
    return static_cast<double>(_total_corpus_terms) / _doc_sizes->size();
}

void inverted_index::chunk_handler::handle_doc(const corpus::document & doc)
{
    for(const auto & count: doc.counts())   // count: (string, double)
    {
        term_id t_id{idx_->get_term_id(count.first)};
        postings_data_type pd{t_id};
        pd.increase_count(doc.id(), count.second);
        auto it = pdata_.find(pd);
        if(it == pdata_.end())
        {
            chunk_size_ += pd.bytes_used();
            pdata_.emplace(pd);
        }
        else
        {
            chunk_size_ -= it->bytes_used();

            // note: we can modify elements in this set because we do not change
            // how comparisons are made (the primary_key value)
            const_cast<postings_data_type &>(*it).merge_with(pd);
            chunk_size_ += it->bytes_used();
        }
        if(chunk_size_ >= max_size())
        {
            idx_->write_chunk(chunk_num_.fetch_add(1), pdata_);
            chunk_size_ = 0;
        }
    }
}

inverted_index::chunk_handler::~chunk_handler() {
    idx_->write_chunk(chunk_num_.fetch_add(1), pdata_);
}

}
}
