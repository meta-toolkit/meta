/**
 * @file default_edge.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DEFAULT_EDGE_H_
#define META_DEFAULT_EDGE_H_

#include "meta/config.h"
#include "meta/meta.h"

namespace meta
{
namespace graph
{
struct default_edge
{
    /**
     * Creates an edge with weight 0
     */
    default_edge() : weight{0.0}
    {
    }

    /**
     * @param w Creates an edge with the weight w
     */
    default_edge(double w) : weight{w}
    {
    }

    /// the weight for this edge
    double weight;
    node_id src;  /// This field must exist in all edge objects.
    node_id dest; /// This field must exist in all edge objects.
};
}
}

#endif
