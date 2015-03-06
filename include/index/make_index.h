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
#include "caching/all.h"
#include "index/cached_index.h"
#include "util/filesystem.h"

namespace meta
{
namespace index
{

class inverted_index;
class forward_index;

/// Inverted index using default DBLRU cache
using dblru_inverted_index =
    cached_index<inverted_index, caching::default_dblru_cache>;

/// Inverted index using splay cache
using splay_inverted_index = cached_index<inverted_index, caching::splay_cache>;

/// In-memory forward index
using memory_forward_index =
    cached_index<forward_index, caching::no_evict_cache>;

/// Forward index using default DBLRU cache
using dblru_forward_index =
    cached_index<forward_index, caching::default_dblru_cache>;

/// Forward index using splay cache
using splay_forward_index = cached_index<forward_index, caching::splay_cache>;

/**
 * Factory method for creating indexes.
 * Usage:
 *
 * ~~~cpp
 * auto idx = index::make_index<derived_index_type>(config_path);
 * ~~~
 *
 * @param config_file The path to the configuration file to be
 *  used to build the index
 * @param args any additional arguments to forward to the
 *  constructor for the chosen index type (usually none)
 * @return A properly initialized index
 */
template <class Index, class... Args>
std::shared_ptr<Index> make_index(const std::string& config_file,
                                  Args&&... args)
{
    auto config = cpptoml::parse_file(config_file);

    // check if we have paths specified for either kind of index
    if (!(config.contains("forward-index")
          && config.contains("inverted-index")))
    {
        throw typename Index::exception{
            "forward-index or inverted-index missing from configuration file"};
    }

    // make sure that the index names are different!
    auto fwd_name = config.get_as<std::string>("forward-index");
    auto inv_name = config.get_as<std::string>("inverted-index");

    if (*fwd_name == *inv_name)
    {
        throw typename Index::exception{
            "forward and inverted index names must be different!"};
    }

    // can't use std::make_shared here since the Index constructor is private
    auto idx =
        std::shared_ptr<Index>{new Index(config, std::forward<Args>(args)...)};

    // if index has already been made, load it
    if (filesystem::make_directory(idx->index_name()) && idx->valid())
        idx->load_index();
    else
        idx->create_index(config_file);

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
    make_index(const std::string& config_file, Args&&... args)
{
    return make_index<cached_index<Index, Cache>>(config_file,
                                                  std::forward<Args>(args)...);
}
}
}

#endif
