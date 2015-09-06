#include "parser/analyzers/featurizers/skeleton_featurizer.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

template <class T>
const std::string skeleton_featurizer<T>::id = "skel";

namespace
{
template <class T>
class skeleton_visitor : public parser::const_visitor<std::string>
{
  public:
    using feature_map = typename skeleton_featurizer<T>::feature_map;

    skeleton_visitor(feature_map& fm) : counts(fm)
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

        counts[rep] += 1;
        return rep;
    }

    std::string operator()(const parser::leaf_node&) override
    {
        std::string rep = "()";
        counts[rep] += 1;
        return rep;
    }

  private:
    feature_map& counts;
};
}

template <class T>
void skeleton_featurizer<T>::tree_tokenize(const parser::parse_tree& tree,
                                           feature_map& counts) const
{
    skeleton_visitor<T> vtor{counts};
    tree.visit(vtor);
}

template class skeleton_featurizer<uint64_t>;
template class skeleton_featurizer<double>;
}
}
