#include "index/cached_index.h"

namespace meta {
namespace index {

template <class Index, template <class, class> class Cache>
template <class... Args>
cached_index<Index, Cache>::cached_index(cpptoml::toml_group & config,
                                         Args &&... args)
    : Index{config}, cache_{std::forward<Args>(args)...} { /* nothing */ }

template <class Index, template <class, class> class Cache>
std::shared_ptr<postings_data<typename Index::primary_key_type, 
                              typename Index::secondary_key_type>>
cached_index<Index, Cache>::search_primary(primary_key_type p_id) const {
    if(cache_.exists(p_id))
        return cache_.find(p_id);
    auto result = Index::search_primary(p_id);
    cache_.insert(p_id, result);
    return result;
}

}
}
