/**
 * @file default_node.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DEFAULT_NODE_H_
#define META_DEFAULT_NODE_H_

#include <string>

#include "meta/config.h"
#include "meta/meta.h"

namespace meta
{
namespace graph
{
struct default_node
{
    /**
     * Creates a node with a blank label
     */
    default_node() : label{""}
    {
    }

    /**
     * @param lbl Creates a node with the label lbl
     */
    default_node(const std::string& lbl) : label{lbl}
    {
    }

    /// the text label for this node
    std::string label;
    node_id id;
};
}
}

#endif
