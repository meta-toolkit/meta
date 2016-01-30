#include "meta/parser/analyzers/featurizers/skeleton_featurizer.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const util::string_view skeleton_featurizer::id = "skel";

namespace
{
class skeleton_visitor : public parser::const_visitor<std::string>
{
  public:
    skeleton_visitor(featurizer& fm) : counts(fm)
    {
        // nothing
    }

    std::string operator()(const parser::internal_node& in) override
    {
        std::string rep = "(";
        in.each_child([&](const parser::node* child)
                      {
                          rep += child->accept(*this);
                      });
        rep += ")";

        counts(rep, 1ul);
        return rep;
    }

    std::string operator()(const parser::leaf_node&) override
    {
        std::string rep = "()";
        counts(rep, 1ul);
        return rep;
    }

  private:
    featurizer& counts;
};
}

void skeleton_featurizer::tree_tokenize(const parser::parse_tree& tree,
                                        featurizer& counts) const
{
    skeleton_visitor vtor{counts};
    tree.visit(vtor);
}
}
}
