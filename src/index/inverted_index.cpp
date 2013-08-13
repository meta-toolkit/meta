/**
 * @file inverted_index.h
 * @author Sean Massung
 */

#include "index/inverted_index.h"

namespace meta {
namespace index {

uint32_t inverted_index::idf(term_id t_id)
{
    postings_data pdata = search_term(t_id);
    return pdata.idf();
}

uint32_t inverted_index::doc_size(doc_id d_id) const
{
    // TODO
    return d_id;
}

uint32_t inverted_index::term_freq(term_id t_id, doc_id d_id)
{
    postings_data pdata = search_term(t_id);
    return pdata.count(d_id);
}

postings_data inverted_index::search_term(term_id t_id)
{
    // TODO
    return postings_data{t_id};
}

}
}
