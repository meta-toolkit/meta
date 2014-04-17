/**
 * @file dblp_node
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_DBLP_NODE_H_
#define META_DBLP_NODE_H_

#include <string>

namespace meta
{
namespace graph
{
struct dblp_node
{
    dblp_node(const std::string& t, const std::string& n): type{t}, name{n} {}

    std::string type;
    std::string name;
};
}
}

#endif
