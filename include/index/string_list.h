/**
 * @file string_list.h
 * @author Chase Geigle
 */

#ifndef _META_STRING_LIST_H_
#define _META_STRING_LIST_H_

#include <string>

#include "io/mmap_file.h"
#include "util/disk_vector.h"

namespace meta
{
namespace index
{

/**
 * A class designed for reading large lists of strings that have been
 * persisted to disk. This class provides read-only
 * access---string_list_writer provides write-only access and is to be used
 * for building the string list and associated index this class reads.
 */
class string_list
{
  public:
    /**
     * Constructs the string list
     */
    string_list(const std::string& path);

    /**
     * Gets the string at a given index.
     */
    const char* at(uint64_t idx) const;

    /**
     * Gets the number of strings in the list.
     */
    uint64_t size() const;

  private:
    /**
     * The file containing the strings.
     */
    io::mmap_file string_file_;

    /**
     * An index that gives the starting byte for each index.
     */
    util::disk_vector<uint64_t> index_;
};
}
}
#endif
