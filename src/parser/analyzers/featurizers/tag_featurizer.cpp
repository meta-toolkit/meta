#include "parser/analyzers/featurizers/tag_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const std::string tag_featurizer::id = "tag";

namespace
{
class tag_visitor : public parser::const_visitor<void>
{
  public:
    tag_visitor(corpus::document& d) : doc(d)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        doc.increment(tag_featurizer::id + "-"
                      + static_cast<std::string>(in.category()),
                      1);
        in.each_child([&](const parser::node* child)
                      {
                          child->accept(*this);
                      });
    }

    void operator()(const parser::leaf_node& ln) override
    {
        doc.increment(tag_featurizer::id + "-"
                      + static_cast<std::string>(ln.category()),
                      1);
    }

  private:
    corpus::document& doc;
};
}

void tag_featurizer::tree_tokenize(corpus::document& doc,
                                   const parser::parse_tree& tree) const
{
    tag_visitor vtor{doc};
    tree.visit(vtor);
}
}
}
