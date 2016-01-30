#include "meta/parser/analyzers/featurizers/subtree_featurizer.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const util::string_view subtree_featurizer::id = "subtree";

namespace
{
class subtree_visitor : public parser::const_visitor<void>
{
  public:
    subtree_visitor(featurizer& fm) : counts(fm)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        auto rep = "(" + static_cast<std::string>(in.category());

        in.each_child(
            [&](const parser::node* child)
            {
                rep += " (" + static_cast<std::string>(child->category()) + ")";
                child->accept(*this);
            });

        rep += ")";
        counts(subtree_featurizer::id.to_string() + "-" + rep, 1ul);
    }

    void operator()(const parser::leaf_node& ln) override
    {
        auto rep = "(" + static_cast<std::string>(ln.category()) + ")";
        counts(subtree_featurizer::id.to_string() + "-" + rep, 1ul);
    }

  private:
    featurizer& counts;
};
}

void subtree_featurizer::tree_tokenize(const parser::parse_tree& tree,
                                       featurizer& counts) const
{
    subtree_visitor vtor{counts};
    tree.visit(vtor);
}
}
}
