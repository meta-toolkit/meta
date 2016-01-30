/**
 * @file cached_index.tcc
 * @author Chase Geigle
 */

#include "meta/index/cached_index.h"

namespace meta
{
namespace index
{

template <class Index, template <class, class> class Cache>
template <class... Args>
cached_index<Index, Cache>::cached_index(const cpptoml::table& config,
                                         Args&&... args)
    : Index{config}, cache_(std::forward<Args>(args)...)
{
    /* nothing */
}

template <class Index, template <class, class> class Cache>
auto cached_index<Index, Cache>::search_primary(primary_key_type p_id) const
    -> std::shared_ptr<postings_data_type>
{
    auto opt = cache_.find(p_id);
    if (opt)
        return *opt;
    auto result = Index::search_primary(p_id);
    cache_.insert(p_id, result);
    return result;
}

template <class Index, template <class, class> class Cache>
void cached_index<Index, Cache>::clear_cache()
{
    cache_.clear();
}
}
}
