#include "meta/parser/analyzers/featurizers/semi_skeleton_featurizer.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const util::string_view semi_skeleton_featurizer::id = "semi-skel";

namespace
{
class semi_skeleton_visitor : public parser::const_visitor<std::string>
{
  public:
    semi_skeleton_visitor(featurizer& fm) : counts(fm)
    {
        // nothing
    }

    std::string operator()(const parser::internal_node& in) override
    {
        auto rep_cat = "(" + static_cast<std::string>(in.category());
        std::string rep;
        in.each_child([&](const parser::node* child)
                      {
                          rep += child->accept(*this);
                      });
        rep += ")";

        counts(semi_skeleton_featurizer::id.to_string() + "-" + rep_cat + rep,
               1ul);
        return "(" + rep;
    }

    std::string operator()(const parser::leaf_node& ln) override
    {
        counts(semi_skeleton_featurizer::id.to_string() + "-("
                   + static_cast<std::string>(ln.category()) + ")",
               1ul);
        return "()";
    }

  private:
    featurizer& counts;
};
}

void semi_skeleton_featurizer::tree_tokenize(const parser::parse_tree& tree,
                                             featurizer& counts) const
{
    semi_skeleton_visitor vtor{counts};
    tree.visit(vtor);
}
}
}
