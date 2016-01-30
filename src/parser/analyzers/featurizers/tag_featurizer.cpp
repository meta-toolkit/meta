#include "meta/parser/analyzers/featurizers/tag_featurizer.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

const util::string_view tag_featurizer::id = "tag";

namespace
{
class tag_visitor : public parser::const_visitor<void>
{
  public:
    tag_visitor(featurizer& fm) : counts(fm)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        counts(tag_featurizer::id.to_string() + "-"
                   + static_cast<std::string>(in.category()),
               1ul);
        in.each_child([&](const parser::node* child)
                      {
                          child->accept(*this);
                      });
    }

    void operator()(const parser::leaf_node& ln) override
    {
        counts(tag_featurizer::id.to_string() + "-"
                   + static_cast<std::string>(ln.category()),
               1ul);
    }

  private:
    featurizer& counts;
};
}

void tag_featurizer::tree_tokenize(const parser::parse_tree& tree,
                                   featurizer& counts) const
{
    tag_visitor vtor{counts};
    tree.visit(vtor);
}
}
}
