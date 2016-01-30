/**
 * @file parser_test.cpp
 * @author Chase Geigle
 * @author Sean Massung
 */

#include <sstream>

#include "bandit/bandit.h"
#include "meta/parser/io/ptb_reader.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/visitors/annotation_remover.h"
#include "meta/parser/trees/visitors/empty_remover.h"
#include "meta/parser/trees/visitors/unary_chain_remover.h"
#include "meta/parser/trees/visitors/multi_transformer.h"
#include "meta/parser/trees/visitors/head_finder.h"
#include "meta/parser/trees/visitors/binarizer.h"
#include "meta/parser/trees/visitors/debinarizer.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

using namespace bandit;
using namespace meta;

namespace {

parser::parse_tree tree(std::string input) {
    std::stringstream in_ss{input};
    auto in_trees = parser::io::extract_trees(in_ss);
    return std::move(in_trees.front());
}

void assert_tree_equal(std::string input, std::string expected,
                       parser::tree_transformer& trns) {
    std::stringstream in_ss{input};
    auto in_trees = parser::io::extract_trees(in_ss);

    in_trees.front().transform(trns);

    std::stringstream exp_ss{expected};
    auto exp_trees = parser::io::extract_trees(exp_ss);
    AssertThat(in_trees.front(), Equals(exp_trees.front()));
}

struct annotation_checker : public parser::const_visitor<bool> {
    bool operator()(const parser::leaf_node&) override {
        return true;
    }

    bool operator()(const parser::internal_node& inode) override {
        if (!inode.head_constituent())
            return false;

        if (!inode.head_lexicon())
            return false;

        bool res = true;
        inode.each_child([&](const parser::node* child) {
            res = res && child->accept(*this);
        });
        return res;
    }
};

struct binary_checker : public parser::const_visitor<bool> {
    bool operator()(const parser::leaf_node&) override {
        return true;
    }

    bool operator()(const parser::internal_node& inode) override {
        if (inode.num_children() > 2)
            return false;

        bool res = true;
        inode.each_child([&](const parser::node* child) {
            res = res && child->accept(*this);
        });
        return res;
    }
};
}

go_bandit([]() {
    using namespace parser;

    describe("[parser] transformer", [&]() {

        annotation_remover ann_remover;
        empty_remover empty_rem;
        unary_chain_remover uchain_rem;

        multi_transformer<annotation_remover, empty_remover,
                          unary_chain_remover> multi;

        it("should remove annotations", [&]() {
            auto tree = "((X (Y (Z-XXX (Y z))) (Z|Q (Y=1 (X x)))))";
            auto tree_noann = "((X (Y (Z (Y z))) (Z (Y (X x)))))";
            assert_tree_equal(tree, tree_noann, ann_remover);
        });

        it("should remove empty nodes", [&]() {
            auto tree = "((X (Y (-NONE- *)) (Z z) (W (Y (-NONE- *) (Q q)))))";
            auto tree_noempty = "((X (Z z) (W (Y (Q q)))))";
            assert_tree_equal(tree, tree_noempty, empty_rem);
        });

        it("should remove unary chains", [&]() {
            auto tree = "((X (X (X (Y y) (Z z)) (X (X (X x))))))";
            auto tree_nochain = "((X (X (Y y) (Z z)) (X x)))";
            assert_tree_equal(tree, tree_nochain, uchain_rem);
        });

        it("should be able to perform multiple transformations", [&]() {
            auto tree
                = "((X (Y-NNN (-NONE- *)) (Z (Z (Z z))) (W (W (Y (-NONE- *) "
                  "(Q q))))))";
            auto tree_trans = "((X (Z z) (W (Y (Q q)))))";
            assert_tree_equal(tree, tree_trans, multi);
        });
    });

    describe("[parser] head finder", [&]() {

        it("should find all annotated heads", [&]() {
            head_finder hf;
            annotation_checker ac;
            auto tr = tree(
                "((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");
            tr.visit(hf);
            AssertThat(tr.visit(ac), IsTrue());
        });
    });

    describe("[parser] binarizer", [&]() {

        head_finder hf;
        binarizer bin;
        binary_checker bin_check;
        annotation_checker ann_check;

        it("should make a binary tree", [&]() {
            auto tr = tree(
                "((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");
            tr.visit(hf);
            tr.transform(bin);
            AssertThat(tr.visit(bin_check), IsTrue());
        });

        it("should keep annotations", [&]() {
            auto tr = tree(
                "((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");
            tr.visit(hf);
            tr.transform(bin);
            AssertThat(tr.visit(ann_check), IsTrue());
        });

        it("should have correct output", [&]() {
            auto tr = tree(
                "((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");
            tr.visit(hf);
            tr.transform(bin);
            auto expected
                = tree("((S (NP (PRP$ My) (NN dog)) (S* (ADVP (RB also)) "
                       "(S* (VP (VBZ likes) (S (VP (VBG eating) (NP (NN "
                       "sausage))))) (. .)))))");
            AssertThat(tr, Equals(expected));
        });
    });

    describe("[parser] debinarizer", [&]() {

        head_finder hf;
        binarizer bin;
        debinarizer debin;
        annotation_checker ann_check;

        it("should debinarize to correct output", [&]() {
            auto tr = tree(
                "((S (S* (NP (PRP$ My) (NN dog)) (S* (ADVP (RB also)) (VP "
                "(VBZ likes) (S (VP (VBG eating) (NP (NN sausage))))))) (. "
                ".)))");
            tr.transform(debin);
            auto expected = tree(
                "((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");
            AssertThat(tr, Equals(expected));
        });

        it("should preserve annotations", [&]() {
            auto tr = tree(
                "((S (NP (PRP$ My) (NN dog)) (ADVP (RB also)) (VP (VBZ "
                "likes) (S (VP (VBG eating) (NP (NN sausage))))) (. .)))");
            tr.visit(hf);
            tr.transform(bin);
            tr.transform(debin);
            AssertThat(tr.visit(ann_check), IsTrue());
        });
    });
});
