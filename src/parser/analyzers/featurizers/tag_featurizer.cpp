#include "parser/analyzers/featurizers/tag_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

template <class T>
const util::string_view tag_featurizer<T>::id = "tag";

namespace
{
template <class T>
class tag_visitor : public parser::const_visitor<void>
{
  public:
    using feature_map = typename tag_featurizer<T>::feature_map;

    tag_visitor(feature_map& fm) : counts(fm)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        counts[tag_featurizer<T>::id.to_string() + "-"
               + static_cast<std::string>(in.category())] += 1;
        in.each_child([&](const parser::node* child)
                      {
                          child->accept(*this);
                      });
    }

    void operator()(const parser::leaf_node& ln) override
    {
        counts[tag_featurizer<T>::id.to_string() + "-"
               + static_cast<std::string>(ln.category())] += 1;
    }

  private:
    feature_map& counts;
};
}

template <class T>
void tag_featurizer<T>::tree_tokenize(const parser::parse_tree& tree,
                                      feature_map& counts) const
{
    tag_visitor<T> vtor{counts};
    tree.visit(vtor);
}

template class tag_featurizer<uint64_t>;
template class tag_featurizer<double>;
}
}
