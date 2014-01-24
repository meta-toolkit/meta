/**
 * @file make_index.h
 * @author Sean Massung
 * @author Chase Geigle
 */

#ifndef _META_MAKE_INDEX_H_
#define _META_MAKE_INDEX_H_

#include "index/cached_index.h"
#include "util/filesystem.h"

namespace meta {
namespace index {

class inverted_index;
class forward_index;

template <class Index, class... Args>
Index make_index(const std::string &config_file, Args &&... args) {
    auto config = cpptoml::parse_file(config_file);

    // check if we have paths specified for either kind of index
    if (!(config.contains("forward-index") &&
          config.contains("inverted-index"))) {
        throw typename Index::exception{
            "forward-index or inverted-index missing from configuration file"};
    }

    Index idx{config, std::forward<Args>(args)...};

    // if index has already been made, load it
    if (filesystem::make_directory(idx._index_name))
        idx.load_index();
    else
        idx.create_index(config_file);

    return idx;
}

template <class Index, template <class, class> class Cache, class... Args>
cached_index<Index, Cache> make_index(const std::string &config_file,
                                      Args &&... args) {
    return make_index<cached_index<Index, Cache>>(config_file,
                                                  std::forward<Args>(args)...);
}
}
}

#endif
