#include "parser/analyzers/featurizers/semi_skeleton_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const std::string semi_skeleton_featurizer::id = "semi-skel";

namespace
{
class semi_skeleton_visitor : public parser::const_visitor<std::string>
{
  public:
    semi_skeleton_visitor(corpus::document& d) : doc(d)
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

        doc.increment(semi_skeleton_featurizer::id + "-" + rep_cat + rep, 1);
        return "(" + rep;
    }

    std::string operator()(const parser::leaf_node& ln) override
    {
        doc.increment(semi_skeleton_featurizer::id + "-("
                      + static_cast<std::string>(ln.category()) + ")",
                      1);
        return "()";
    }

  private:
    corpus::document& doc;
};
}

void semi_skeleton_featurizer::tree_tokenize(
    corpus::document& doc, const parser::parse_tree& tree) const
{
    semi_skeleton_visitor vtor{doc};
    tree.visit(vtor);
}
}
}
