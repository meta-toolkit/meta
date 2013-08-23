/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "index/forward_index.h"
#include "index/chunk.h"
#include "io/config_reader.h"

using std::cerr;
using std::endl;

namespace meta {
namespace index {

forward_index::forward_index(const std::string & index_name,
                             std::vector<document> & docs,
                             std::shared_ptr<tokenizers::tokenizer> & tok,
                             const std::string & config_file):
    disk_index(index_name, tok)
{
    create_index(docs, config_file);
}

forward_index::forward_index(const std::string & index_name):
    disk_index(index_name)
{ /* nothing */ }

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
        _doc_id_mapping[doc_num] = doc.name();
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

uint64_t forward_index::term_freq(term_id t_id, doc_id d_id)
{
    postings_data<doc_id, term_id> pdata = search_term(d_id);
    return pdata.count(t_id);
}

const std::unordered_map<term_id, uint64_t> forward_index::counts(doc_id d_id)
{
    postings_data<doc_id, term_id> pdata = search_term(d_id);
    return pdata.counts();
}

}
}
