/**
 * @file vocabulary_map.h
 * @author Chase Geigle
 */

#ifndef _META_VOCABULARY_MAP_H_
#define _META_VOCABULARY_MAP_H_

#include "io/mmap_file.h"
#include "util/disk_vector.h"
#include "util/optional.h"

namespace meta
{
namespace index
{

class vocabulary_map
{
  private:
    io::mmap_file file_;
    util::disk_vector<uint64_t> inverse_;
    uint64_t block_size_;
    uint64_t leaf_end_pos_;
    uint64_t initial_seek_pos_;

    int compare(const std::string & term, const char * other) const;

  public:
    vocabulary_map(const std::string & path, uint16_t block_size = 4096);
    util::optional<term_id> find(const std::string & term) const;
    std::string find_term(term_id t_id) const;
    uint64_t size() const;
};
}
}

#endif
