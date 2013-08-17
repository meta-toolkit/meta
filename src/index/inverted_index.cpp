/**
 * @file inverted_index.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "index/inverted_index.h"
#include "index/chunk.h"
#include "io/config_reader.h"

using std::cerr;
using std::endl;

namespace meta {
namespace index {

inverted_index::inverted_index(const std::string & index_name,
                               std::vector<document> & docs,
                               std::shared_ptr<tokenizers::tokenizer> & tok,
                               const std::string & config_file):
    disk_index(index_name, docs, tok, config_file),
    _avg_dl(-1.0)
{ /* nothing */ }

inverted_index::inverted_index(const std::string & index_name):
    disk_index(index_name),
    _avg_dl(-1.0)
{ /* nothing */ }

uint32_t inverted_index::tokenize_docs(std::vector<document> & docs)
{
    std::unordered_map<term_id, postings_data<term_id, doc_id>> pdata;
    uint32_t chunk_num = 0;
    uint64_t doc_num = 0;
    std::string progress = "Tokenizing ";
    for(auto & doc: docs)
    {
        common::show_progress(doc_num, docs.size(), 20, progress);
        _tokenizer->tokenize(doc);
        _doc_id_mapping[doc_num] = doc.name();
        _doc_sizes[doc_num] = doc.length();

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

uint64_t inverted_index::idf(term_id t_id)
{
    postings_data<term_id, doc_id> pdata = search_term(t_id);
    return pdata.inverse_frequency();
}

uint64_t inverted_index::term_freq(term_id t_id, doc_id d_id)
{
    postings_data<term_id, doc_id> pdata = search_term(t_id);
    return pdata.count(d_id);
}

const std::unordered_map<doc_id, uint64_t> inverted_index::counts(term_id t_id)
{
    postings_data<term_id, doc_id> pdata = search_term(t_id);
    return pdata.counts();
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
