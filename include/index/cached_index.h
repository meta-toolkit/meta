/**
 * @file disk_index.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Chase Geigle
 */

#ifndef _CACHED_INDEX_H_
#define _CACHED_INDEX_H_

#include "index/document.h"
#include "index/postings_data.h"
#include "meta.h"

namespace meta {
namespace index {

template <class Index, template <class, class> class Cache>
class cached_index : public Index {
    public:
        // inherit the constructors
        using Index::Index;

        template <class... Args>
        cached_index(cpptoml::toml_group & config, Args &&... args);

        using primary_key_type   = typename Index::primary_key_type;
        using secondary_key_type = typename Index::secondary_key_type;

        virtual std::shared_ptr<postings_data<primary_key_type, 
                                              secondary_key_type>> 
        search_primary(primary_key_type p_id) const override;
    private:
        mutable Cache<primary_key_type, 
                      std::shared_ptr<postings_data<primary_key_type, 
                                                    secondary_key_type>
                      >
                > cache_;
};

}
}

#include "index/cached_index.tcc"
#endif
