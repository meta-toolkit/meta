/**
 * @file make_index.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_MAKE_INDEX_H_
#define META_MAKE_INDEX_H_

#include "cpptoml.h"
#include "meta/caching/all.h"
#include "meta/config.h"
#include "meta/corpus/corpus_factory.h"
#include "meta/index/cached_index.h"
#include "meta/io/filesystem.h"

namespace meta
{
namespace index
{

class inverted_index;
class forward_index;

/// Inverted index using default DBLRU cache
using dblru_inverted_index
    = cached_index<inverted_index, caching::default_dblru_cache>;

/// Inverted index using splay cache
using splay_inverted_index = cached_index<inverted_index, caching::splay_cache>;

/// In-memory forward index
using memory_forward_index
    = cached_index<forward_index, caching::no_evict_cache>;

/// Forward index using default DBLRU cache
using dblru_forward_index
    = cached_index<forward_index, caching::default_dblru_cache>;

/// Forward index using splay cache
using splay_forward_index = cached_index<forward_index, caching::splay_cache>;

/**
 * Factory method for creating indexes.
 * Usage:
 *
 * ~~~cpp
 * auto idx = index::make_index<derived_index_type>(config);
 * ~~~
 *
 * @param config The configuration to be used to build the index
 * @param corpus The collection of documents to index
 * @param args any additional arguments to forward to the
 *  constructor for the chosen index type (usually none)
 * @return A properly initialized index
 */
template <class Index, class... Args>
std::shared_ptr<Index> make_index(const cpptoml::table& config,
                                  corpus::corpus& docs, Args&&... args)
{
    if (!config.contains("index"))
    {
        throw typename Index::exception{
            "index name missing from configuration file"};
    }

    // below is needed so that make_shared can find a public ctor to invoke
    struct make_shared_enabler : public Index
    {
        make_shared_enabler(const cpptoml::table& config, Args&&... args)
            : Index(config, std::forward<Args>(args)...)
        {
            // nothing
        }
    };

    auto idx = std::make_shared<make_shared_enabler>(
        config, std::forward<Args>(args)...);

    // if index has already been made, load it
    if (filesystem::exists(idx->index_name()) && idx->valid())
    {
        idx->load_index();
    }
    else
    {
        filesystem::remove_all(idx->index_name());
        idx->create_index(config, docs);
    }

    return idx;
}

/**
 * Helper for make_index that creates a corpus from the global config file.
 */
template <class Index, class... Args>
std::shared_ptr<Index> make_index(const cpptoml::table& config, Args&&... args)
{
    if (!config.contains("index"))
    {
        throw typename Index::exception{
            "index name missing from configuration file"};
    }

    // below is needed so that make_shared can find a public ctor to invoke
    struct make_shared_enabler : public Index
    {
        make_shared_enabler(const cpptoml::table& config, Args&&... args)
            : Index(config, std::forward<Args>(args)...)
        {
            // nothing
        }
    };

    auto idx = std::make_shared<make_shared_enabler>(
        config, std::forward<Args>(args)...);

    // if index has already been made, load it
    if (filesystem::exists(idx->index_name()) && idx->valid())
    {
        idx->load_index();
    }
    else
    {
        filesystem::remove_all(idx->index_name());
        auto docs = corpus::make_corpus(config);
        idx->create_index(config, *docs);
    }

    return idx;
}

/**
 * Factory method for creating indexes that are cached.
 * Usage:
 *
 * ~~~cpp
 * auto idx =
 *     index::make_index<dervied_index_type,
 *                       cache_type>(config_path, other, options);
 * ~~~
 *
 * Other options will be forwarded to the constructor for the
 * chosen cache class.
 *
 * @param config_file the path to the configuration file to be
 *  used to build the index.
 * @param args any additional arguments to forward to the
 *  constructor for the cache class chosen
 * @return A properly initialized, and automatically cached, index.
 */
template <class Index, template <class, class> class Cache, class... Args>
std::shared_ptr<cached_index<Index, Cache>>
make_index(const cpptoml::table& config, Args&&... args)
{
    return make_index<cached_index<Index, Cache>>(config,
                                                  std::forward<Args>(args)...);
}
}
}

#endif
