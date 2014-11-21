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

void assert_tree_equal(std::string input, std::string expected,
                       parser::tree_transformer& trns)
{
    std::stringstream in_ss{input};
    auto in_trees = parser::io::extract_trees(in_ss);

    in_trees.front().transform(trns);

    std::stringstream exp_ss{expected};
    auto exp_trees = parser::io::extract_trees(exp_ss);

    ASSERT(in_trees.front() == exp_trees.front());
}

int transformer_tests()
{
    using namespace parser;

    auto num_failed = int{0};

    annotation_remover ann_remover;
    empty_remover empty_rem;
    unary_chain_remover uchain_rem;

    num_failed += testing::run_test("annotation_remover", [&]()
                                    {
        auto tree = "((X (Y (Z-XXX (Y z))) (Z|Q (Y=1 (X x)))))";
        auto tree_noann = "((X (Y (Z (Y z))) (Z (Y (X x)))))";

        assert_tree_equal(tree, tree_noann, ann_remover);
    });

    num_failed += testing::run_test("empty_remover", [&]()
                                    {
        auto tree = "((X (Y (-NONE- *)) (Z z) (W (Y (-NONE- *) (Q q)))))";
        auto tree_noempty = "((X (Z z) (W (Y (Q q)))))";

        assert_tree_equal(tree, tree_noempty, empty_rem);
    });

    num_failed += testing::run_test("unary_chain_remover", [&]()
                                    {
        auto tree = "((X (X (X (Y y) (Z z)) (X (X (X x))))))";
        auto tree_nochain = "((X (X (Y y) (Z z)) (X x)))";

        assert_tree_equal(tree, tree_nochain, uchain_rem);
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
