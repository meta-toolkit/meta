#include "parser/analyzers/featurizers/subtree_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

template <class T>
const std::string subtree_featurizer<T>::id = "subtree";

namespace
{
template <class T>
class subtree_visitor : public parser::const_visitor<void>
{
  public:
    using feature_map = typename subtree_featurizer<T>::feature_map;

    subtree_visitor(feature_map& fm) : counts(fm)
    {
        // nothing
    }

    void operator()(const parser::internal_node& in) override
    {
        auto rep = "(" + static_cast<std::string>(in.category());

        in.each_child(
            [&](const parser::node* child)
            {
                rep += " (" + static_cast<std::string>(child->category()) + ")";
                child->accept(*this);
            });

        rep += ")";
        counts[subtree_featurizer<T>::id + "-" + rep] += 1;
    }

    void operator()(const parser::leaf_node& ln) override
    {
        auto rep = "(" + static_cast<std::string>(ln.category()) + ")";
        counts[subtree_featurizer<T>::id + "-" + rep] += 1;
    }

  private:
    feature_map& counts;
};
}

template <class T>
void subtree_featurizer<T>::tree_tokenize(const parser::parse_tree& tree,
                                          feature_map& counts) const
{
    subtree_visitor<T> vtor{counts};
    tree.visit(vtor);
}

template class subtree_featurizer<uint64_t>;
template class subtree_featurizer<double>;
}
}
