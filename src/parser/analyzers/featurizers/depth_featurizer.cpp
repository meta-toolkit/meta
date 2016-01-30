#include "meta/parser/analyzers/featurizers/depth_featurizer.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const util::string_view depth_featurizer::id = "depth";

namespace
{
class height_visitor : public parser::const_visitor<size_t>
{
  public:
    size_t operator()(const parser::internal_node& in) override
    {
        size_t max_height = 0;
        in.each_child([&](const parser::node* child)
                      {
                          max_height
                              = std::max(max_height, child->accept(*this));
                      });
        return max_height + 1;
    }

    size_t operator()(const parser::leaf_node&) override
    {
        // leaf nodes are pre-terminals
        return 1;
    }
};
}

void depth_featurizer::tree_tokenize(const parser::parse_tree& tree,
                                     featurizer& counts) const
{
    height_visitor vtor;
    auto rep = "depth-" + std::to_string(tree.visit(vtor));
    counts(rep, 1ul);
}
}
}
