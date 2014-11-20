/**
 * @file parser_test.cpp
 * @author Chase Geigle
 */

#include <sstream>
#include "test/parser_test.h"
#include "test/unit_test.h"
#include "parser/io/ptb_reader.h"
#include "parser/trees/transformers/annotation_remover.h"
#include "parser/trees/transformers/empty_remover.h"
#include "parser/trees/transformers/unary_chain_remover.h"

namespace meta
{
namespace testing
{

void transform(parser::parse_tree&)
{
    return;
}

template <class Head, class... Rest>
void transform(parser::parse_tree& tree, Head&& head, Rest&&... rest)
{
    tree.transform(head);
    transform(tree, rest...);
}

template <class... Transformers>
void assert_tree_equal(std::string input, std::string expected,
                        Transformers&&... trans)
{
    std::stringstream in_ss{input};
    auto in_trees = parser::io::extract_trees(in_ss);

    transform(in_trees.front(), trans...);

    std::stringstream exp_ss{expected};
    auto exp_trees = parser::io::extract_trees(exp_ss);

    ASSERT(in_trees.front() == exp_trees.front());
}

int transformer_tests()
{
    using namespace parser;

    auto num_failed = int{0};

    annotation_remover ann_remover;

    num_failed += testing::run_test("annotation_remover_basic", [&]()
                                    {
        auto tree = "((X (Y (Z-XXX (Y z))) (Z|Q (Y=1 (X x)))))";
        auto tree_noann = "((X (Y (Z     (Y z))) (Z   (Y   (X x)))))";

        assert_tree_equal(tree, tree_noann, ann_remover);
    });

    return num_failed;
}

int parser_tests()
{
    auto num_failed = int{0};
    num_failed += transformer_tests();
    return num_failed;
}
}
}
