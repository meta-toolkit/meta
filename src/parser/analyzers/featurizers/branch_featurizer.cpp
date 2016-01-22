#include "meta/parser/analyzers/featurizers/branch_featurizer.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const util::string_view branch_featurizer::id = "branch";

namespace
{
class branch_visitor : public parser::const_visitor<void>
{
  public:
    branch_visitor(featurizer& fm) : counts(fm)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        auto rep = "branch-" + std::to_string(in.num_children());
        counts(rep, 1ul);

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
    featurizer& counts;
};
}

void branch_featurizer::tree_tokenize(const parser::parse_tree& tree,
                                      featurizer& counts) const
{
    branch_visitor vtor{counts};
    tree.visit(vtor);
}
}
}
