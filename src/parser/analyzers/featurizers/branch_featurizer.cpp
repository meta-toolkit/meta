#include "parser/analyzers/featurizers/branch_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

template <class T>
const std::string branch_featurizer<T>::id = "branch";

namespace
{
template <class T>
class branch_visitor : public parser::const_visitor<void>
{
  public:
    using feature_map = typename branch_featurizer<T>::feature_map;

    branch_visitor(feature_map& fm) : counts(fm)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        auto rep = "branch-" + std::to_string(in.num_children());
        counts[rep] += 1;

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
    feature_map& counts;
};
}

template <class T>
void branch_featurizer<T>::tree_tokenize(const parser::parse_tree& tree,
                                         feature_map& counts) const
{
    branch_visitor<T> vtor{counts};
    tree.visit(vtor);
}

template class branch_featurizer<uint64_t>;
template class branch_featurizer<double>;
}
}
