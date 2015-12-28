#include "meta/parser/analyzers/featurizers/semi_skeleton_featurizer.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace analyzers
{

template <class T>
const util::string_view semi_skeleton_featurizer<T>::id = "semi-skel";

namespace
{
template <class T>
class semi_skeleton_visitor : public parser::const_visitor<std::string>
{
  public:
    using feature_map = typename semi_skeleton_featurizer<T>::feature_map;

    semi_skeleton_visitor(feature_map& fm) : counts(fm)
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

        counts[semi_skeleton_featurizer<T>::id.to_string() + "-" + rep_cat
               + rep] += 1;
        return "(" + rep;
    }

    std::string operator()(const parser::leaf_node& ln) override
    {
        counts[semi_skeleton_featurizer<T>::id.to_string() + "-("
               + static_cast<std::string>(ln.category()) + ")"] += 1;
        return "()";
    }

  private:
    feature_map& counts;
};
}

template <class T>
void semi_skeleton_featurizer<T>::tree_tokenize(const parser::parse_tree& tree,
                                                feature_map& counts) const
{
    semi_skeleton_visitor<T> vtor{counts};
    tree.visit(vtor);
}

template class semi_skeleton_featurizer<uint64_t>;
template class semi_skeleton_featurizer<double>;
}
}
