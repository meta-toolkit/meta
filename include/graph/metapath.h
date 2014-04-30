/**
 * @file metapath.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GRAPH_METAPTAH_H_
#define META_GRAPH_METAPTAH_H_

#include <string>
#include <stdexcept>
#include <vector>

namespace meta
{
namespace graph
{

/**
 * A metapath is a path from a node to another node in a heterogeneous
 * information network. This path may contain nodes of various types and edges
 * representing various relations.
 */
class metapath
{
  public:
    /**
     * Shows how to transition from one node type to the next node type.
     */
    enum class direction
    {
        forward,  // P -> P, a paper cites another paper
        backward, // P <- P, a paper is cited by another paper
        none      // A -- P, an author writes a paper
    };

    /**
     * Constructor.
     * @param str The string representation of the metapath.
     * The string must contain whitespace delimited nodes with edges between
     * each node. Typically, a node is a capital letter. Edges are either "--",
     * "->", or "<-" to represent directions.
     */
    metapath(const std::string& str);

    /**
     * @param idx
     * @return the node at the specified index in this metapath
     */
    const std::string& operator[](uint64_t idx) const;

    /**
     * @param idx
     * @return the direction the metapath proceeds
     */
    direction edge_dir(uint64_t idx) const;

    /**
     * @return the number of nodes in this metapath
     */
    uint64_t size() const;

  private:
    /// The metapath labels
    std::vector<std::string> path_;

    /// The metapath transisitions
    std::vector<direction> trans_;
};

/**
 * Simple exception for metapath interactions.
 */
class metapath_exception : std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
#endif
