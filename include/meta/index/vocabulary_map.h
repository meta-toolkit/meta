/**
 * @file vocabulary_map.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_VOCABULARY_MAP_H_
#define META_VOCABULARY_MAP_H_

#include "meta/config.h"
#include "meta/io/mmap_file.h"
#include "meta/util/disk_vector.h"
#include "meta/util/optional.h"

namespace meta
{
namespace index
{

/**
 * A read-only view of a B+-tree-like structure that stores the vocabulary
 * for an index. It reads the file format that is written by the
 * vocabulary_map_writer class (see the documentation for the writer for
 * information about the file format).
 */
class vocabulary_map
{
  private:
    /**
     * The file containing the tree. mmapped for performance.
     */
    io::mmap_file file_;

    /**
     * Byte positions for each term in the leaves to allow for reverse
     * lookup of a the string associated with a given id.
     */
    util::disk_vector<uint64_t> inverse_;

    /**
     * The size of the nodes in the tree.
     */
    uint64_t block_size_;

    /**
     * The ending position of the leaf nodes. Used to determine when to
     * stop a search.
     */
    uint64_t leaf_end_pos_;

    /**
     * The position of the first internal node that is not the root. Used
     * to seek to the first level during search.
     */
    uint64_t initial_seek_pos_;

    /**
     * Convenience wrapper for comparing the term with strings in the tree.
     * @param term  the term we are looking for
     * @param other the string in the tree we are considering
     */
    int compare(const std::string& term, const char* other) const;

  public:
    /**
     * Creates a vocabulary map reading the file in the given path with
     * the given block size. Changing the block size is not
     * recommended---the block size used should always be the same as the
     * block size used in the vocabulary_map_writer used to create the
     * tree.
     *
     * @param path the location of the tree file
     * @param block_size the size of the nodes in the tree
     */
    vocabulary_map(const std::string& path, uint16_t block_size = 4096);

    /**
     * Move constructs a vocabulary_map.
     */
    vocabulary_map(vocabulary_map&&) = default;

    /**
     * Move assigns a vocabulary_map.
     */
    vocabulary_map& operator=(vocabulary_map&&) = default;

    /**
     * Finds the given term in the tree, if it exists.
     * @param term the term to find an id for
     */
    util::optional<term_id> find(const std::string& term) const;

    /**
     * Finds the term associated with the given id. No bounds checking is
     * performed---accessing beyond the maximum assigned term_id is
     * undefined behavior.
     *
     * @param t_id the term id to find the string representation of
     */
    std::string find_term(term_id t_id) const;

    /**
     * The number of terms in the map.
     */
    uint64_t size() const;
};
}
}

#endif
