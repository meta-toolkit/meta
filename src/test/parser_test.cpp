/**
 * @file parser_test.cpp
 * @author Chase Geigle
 */

#include <sstream>
#include "test/parser_test.h"
#include "test/unit_test.h"
#include "parser/io/ptb_reader.h"
#include "parser/trees/visitors/visitor.h"
#include "parser/trees/visitors/annotation_remover.h"
#include "parser/trees/visitors/empty_remover.h"
#include "parser/trees/visitors/unary_chain_remover.h"
#include "parser/trees/visitors/multi_transformer.h"
#include "parser/trees/visitors/head_finder.h"
#include "parser/trees/visitors/binarizer.h"
#include "parser/trees/visitors/debinarizer.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace testing
{

parser::parse_tree tree(std::string input)
{
    std::stringstream in_ss{input};
    auto in_trees = parser::io::extract_trees(in_ss);
    return std::move(in_trees.front());
}

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

    multi_transformer<annotation_remover, empty_remover, unary_chain_remover>
        multi;

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

    num_failed += testing::run_test("multi_transformer", [&]()
                                    {
        auto tree = "((X (Y-NNN (-NONE- *)) (Z (Z (Z z))) (W (W (Y (-NONE- *) "
                    "(Q q))))))";
        auto tree_trans = "((X (Z z) (W (Y (Q q)))))";

        assert_tree_equal(tree, tree_trans, multi);
    });

    return num_failed;
}

struct annotation_checker : public parser::const_visitor<bool>
{
    bool operator()(const parser::leaf_node&) override
    {
        return true;
    }

    bool operator()(const parser::internal_node& inode) override
    {
        if (!inode.head_constituent())
            return false;

        if (!inode.head_lexicon())
            return false;

        bool res = true;
        inode.each_child([&](const parser::node* child)
                         {
                             res = res && child->accept(*this);
                         });
        return res;
    }
};

struct binary_checker : public parser::const_visitor<bool>
{
    bool operator()(const parser::leaf_node&) override
    {
        return true;
    }

    bool operator()(const parser::internal_node& inode) override
    {
        if (inode.num_children() > 2)
            return false;

        bool res = true;
        inode.each_child([&](const parser::node* child)
                         {
                             res = res && child->accept(*this);
                         });
        return res;
    }
};

int head_finder_tests()
{
    using namespace parser;

    auto num_failed = int{0};

    num_failed += testing::run_test("head_finder_all_annotated", [&]()
                                    {
        head_finder hf;
        annotation_checker ac;
        auto tr
            = tree("((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                   "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");
        tr.visit(hf);

        ASSERT(tr.visit(ac));
    });

    return num_failed;
}

int binarizer_tests()
{
    using namespace parser;

    auto num_failed = int{0};

    head_finder hf;
    binarizer bin;
    binary_checker bin_check;
    annotation_checker ann_check;

    num_failed += testing::run_test("binarizer_is_binary", [&]()
                                    {
        auto tr
            = tree("((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                   "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");

        tr.visit(hf);
        tr.transform(bin);
        ASSERT(tr.visit(bin_check));
    });

    num_failed += testing::run_test("binarizer_keeps_annotations", [&]()
                                    {
        auto tr
            = tree("((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                   "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");

        tr.visit(hf);
        tr.transform(bin);
        ASSERT(tr.visit(ann_check));
    });

    num_failed += testing::run_test("binarizer_correct_output", [&]()
                                    {
        auto tr
            = tree("((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                   "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");

        tr.visit(hf);
        tr.transform(bin);

        auto expected = tree("((S (NP (PRP$ My) (NN dog)) (S* (ADVP (RB also)) "
                             "(S* (VP (VBZ likes) (S (VP (VBG eating) (NP (NN "
                             "sausage))))) (. .)))))");

        ASSERT(tr == expected);
    });

    return num_failed;
}

int debinarizer_tests()
{
    using namespace parser;

    auto num_failed = int{0};

    head_finder hf;
    binarizer bin;
    debinarizer debin;
    annotation_checker ann_check;

    num_failed += testing::run_test("debinarizer_correct_output", [&]()
                                    {
        auto tr = tree(
            "((S (S* (NP (PRP$ My) (NN dog)) (S* (ADVP (RB also)) (VP "
            "(VBZ likes) (S (VP (VBG eating) (NP (NN sausage))))))) (. .)))");

        tr.transform(debin);

        auto expected
            = tree("((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                   "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");

        ASSERT(tr == expected);
    });

    num_failed += testing::run_test("debinarizer_preserves_annotations", [&]()
                                    {
        auto tr
            = tree("((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                   "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");

        tr.visit(hf);
        tr.transform(bin);
        tr.transform(debin);

        ASSERT(tr.visit(ann_check));
    });

    return num_failed;
}

int parser_tests()
{
    logging::set_cerr_logging();
    auto num_failed = int{0};
    num_failed += transformer_tests();
    num_failed += head_finder_tests();
    num_failed += binarizer_tests();
    num_failed += debinarizer_tests();
    return num_failed;
}
}
}
