#include "parser/analyzers/featurizers/branch_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const std::string branch_featurizer::id = "branch";

namespace
{
class branch_visitor : public parser::const_visitor<void>
{
  public:
    branch_visitor(corpus::document& d) : doc(d)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        auto rep = "branch-" + std::to_string(in.num_children());
        doc.increment(rep, 1);

        in.each_child([&](const parser::node* child)
                      {
                          child->accept(*this);
                      });
    }

    void operator()(const parser::leaf_node&) override
    {
        // nothing
    }

  private:
    corpus::document& doc;
};
}

void branch_featurizer::tree_tokenize(corpus::document& doc,
                                      const parser::parse_tree& tree) const
{
    branch_visitor vtor{doc};
    tree.visit(vtor);
}
}
}
