/**
 * @file inverted_index.cpp
 * @author Sean Massung
 */
#include <iostream>
#include "index/inverted_index.h"
#include "index/chunk.h"

#include <cassert>

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
    auto time = common::time([&]() {
        auto it = doc.counts().begin();
        auto end = doc.counts().end();
        while (it != end) {
        /*
        for(const auto & count: doc.counts())   // count: (string, double)
        {
        */
            const auto & count = *it;
            term_id t_id{0};
            auto time = common::time<std::chrono::microseconds>([&]() {
                t_id = term_id{idx_->get_term_id(count.first)};
            });
            find_term_id_time_ += time.count();

            time = common::time<std::chrono::microseconds>([&]() {
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
                    auto time = common::time<std::chrono::microseconds>([&]() {
                        //const_cast<postings_data_type &>(*it).merge_with(pd);
                        const_cast<postings_data_type &>(*it).increase_count(doc.id(), count.second);
                    });
                    merging_with_time_ += time.count();
                    chunk_size_ += it->bytes_used();
                }
            });
            merging_pdata_time_ += time.count();

            time = common::time<std::chrono::microseconds>([&]() {
                if(chunk_size_ >= max_size())
                {
                    idx_->write_chunk(chunk_num_.fetch_add(1), pdata_);
                    chunk_size_ = 0;
                }
            });
            writing_chunk_time_ += time.count();
            time = common::time<std::chrono::microseconds>([&]() {
                ++it;
            });
            total_iteration_time_ += time.count();
        }
    });
    total_time_ += time.count();
}

inverted_index::chunk_handler::~chunk_handler() {
    /*
    auto time = common::time([&]() {
        idx_->write_chunk(chunk_num_.fetch_add(1), pdata_);
    });
    writing_chunk_time_ += time.count();
    */
}

void inverted_index::chunk_handler::print_stats_impl() {
    auto time = common::time([&]() {
        idx_->write_chunk(chunk_num_.fetch_add(1), pdata_);
    });
    writing_chunk_time_ += time.count();
    total_time_ += time.count();
    std::cerr << "     ! CPU Time finding ids: " << find_term_id_time_ / 1000.0 << "ms" << std::endl;
    std::cerr << "     ! CPU Time merging pdata: " << merging_pdata_time_ / 1000.0 << "ms" << std::endl;
    std::cerr << "       ! CPU Time merge_with: " << merging_with_time_ / 1000.0 << "ms" << std::endl;
    std::cerr << "     ! CPU Time writing chunks: " << writing_chunk_time_ / 1000.0 << "ms" << std::endl;
    std::cerr << "     ! Total CPU time: " << total_time_ / 1000.0 << "s" << std::endl;
    std::cerr << "     ! Iteration overhead: " << total_iteration_time_ / 1000.0 << "ms" << std::endl;
}

}
}
