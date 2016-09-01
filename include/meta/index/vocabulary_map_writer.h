/**
 * @file vocabulary_map_writer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_VOCABULARY_MAP_WRITER_H_
#define META_VOCABULARY_MAP_WRITER_H_

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

#include "meta/config.h"

namespace meta
{
namespace index
{

/**
 * A class that writes the B+-tree-like data structure used for storing
 * the term id mapping in an index. This class provides a write-only view
 * of the mapping.
 *
 * The file format consists of two files: one for the "forward" mapping and
 * one for the "backward" mapping.
 *
 * The "forward" mapping is a structure like a B+-tree, with the exception
 * that each node has a fixed size instead of there always being a fixed
 * number of elements. This is necessary due to the size of the keys being
 * unknown ahead of time; this also allows for some gains in space
 * utilization as we do not pad the keys to be a fixed maximum length.
 * However, the nodes are always block_size bytes large and will be padded
 * with null bytes when needed.
 *
 * Each internal node is a sorted list of strings and byte
 * positions---depending on the string comparison, the search will pick the
 * appropriate byte position and seek there, recursively. The leaf nodes
 * are a sorted list of strings and id assignments. The search linearly
 * scans the leaf node it arrives at for the term in question.
 *
 * The "backward" mapping is simply a disk-persisted vector of byte
 * positions, indexed by the term id. Reading a string from the forward
 * mapping's file starting at the given byte position will yield the string
 * for that id.
 *
 * The mappings created are non-portable and depend on the endianness of
 * the system building them. This may be changed in the future.
 *
 * *This class is not internally synchronized*, so external synchronization
 * must be provided if using it in a threaded context to avoid race
 * conditions.
 */
class vocabulary_map_writer
{
  public:
    /**
     * Creates a writer for a tree at the given path and block_size.
     * Changing the block size is not recommended---if doing so, ensure
     * that any and all vocabulary_maps created for the written file are
     * also created with the same modified block size.
     *
     * @param path the path to the tree file tow rite
     * @param block_size the size of the nodes in the tree
     */
    vocabulary_map_writer(const std::string& path, uint16_t block_size = 4096);

    /**
     * The destructor for a vocabulary_map_writer flushes the last leaf
     * node and builds the internal structure---as such, it may block for
     * a period of time while finalizing the tree structure. This may be
     * changed in the future.
     */
    ~vocabulary_map_writer();

    /**
     * Inserts this term into the map. No checking is done for duplicate
     * terms or that the ordering is indeed lexicographically sorted, but
     * these invariants are important to maintain for the tree to work
     * properly.
     *
     * @param term the term to insert
     */
    void insert(const std::string& term);

  private:
    /**
     * Writes null bytes to fill up the current block.
     */
    void write_padding();

    /**
     * Flushes a node to disk after writing the padding bytes.
     */
    void flush();

    /// The file containing the forward mapping tree.
    std::ofstream file_;

    /**
     * The current write position in the forward mapping tree file. This is
     * kept to avoid potential 32-bit system issues with the
     * `tellp()/tellg()` functions on the standard streams.
     */
    uint64_t file_write_pos_;

    /// The file containing the reverse mapping
    std::ofstream inverse_file_;

    /// The path to the tree file
    std::string path_;

    /// The block size of every node in the tree, in bytes
    uint16_t block_size_;

    /// The total number of terms inserted so far
    uint64_t num_terms_;

    /// The remaining space in the block currently being written
    uint16_t remaining_block_space_;

    /// Number of written nodes to be "merged" when writing the next level
    uint64_t written_nodes_;
};

/**
 * An exception that can be thrown during the building of the tree.
 */
class vocabulary_map_writer_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
}
#endif
