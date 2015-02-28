#include "parser/analyzers/featurizers/subtree_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const std::string subtree_featurizer::id = "subtree";

namespace
{
class subtree_visitor : public parser::const_visitor<void>
{
  public:
    subtree_visitor(corpus::document& d) : doc(d)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        auto rep = "(" + static_cast<std::string>(in.category());

        in.each_child([&](const parser::node* child)
        {
            rep += " (" + static_cast<std::string>(child->category()) + ")";
            child->accept(*this);
        });

        rep += ")";
        doc.increment(subtree_featurizer::id + "-" + rep, 1);
    }

    void operator()(const parser::leaf_node& ln) override
    {
        auto rep = "(" + static_cast<std::string>(ln.category()) + ")";
        doc.increment(subtree_featurizer::id + "-" + rep, 1);
    }


  private:
    corpus::document& doc;
};
}

void subtree_featurizer::tree_tokenize(corpus::document& doc,
                                       const parser::parse_tree& tree) const
{
    subtree_visitor vtor{doc};
    tree.visit(vtor);
}
}
}
