#include "parser/analyzers/featurizers/skeleton_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const std::string skeleton_featurizer::id = "skel";

namespace
{
class skeleton_visitor : public parser::const_visitor<std::string>
{
  public:
    skeleton_visitor(corpus::document& d) : doc(d)
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

        doc.increment(rep, 1);
        return rep;
    }

    std::string operator()(const parser::leaf_node&) override
    {
        std::string rep = "()";
        doc.increment(rep, 1);
        return rep;
    }
  private:
    corpus::document& doc;
};
}

void skeleton_featurizer::tree_tokenize(corpus::document& doc,
                                        const parser::parse_tree& tree) const
{
    skeleton_visitor vtor{doc};
    tree.visit(vtor);
}
}
}
