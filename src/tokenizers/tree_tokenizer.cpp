/**
 * @file tree_tokenizer.cpp
 */

#include <utility>
#include "util/common.h"
#include "index/document.h"
#include "tokenizers/parse_tree.h"
#include "tokenizers/tree_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::vector;
using std::unordered_map;
using std::pair;
using std::bind;
using std::shared_ptr;
using std::string;

using index::Document;
using index::TermID;

tree_tokenizer::tree_tokenizer(TreeTokenizerType type):
    _type(type),
    _tokenizer_types(unordered_map<TreeTokenizerType, TokenizerFunction, std::hash<int>>())
{
    // Bind the member tokenizer functions to the current object, and use them as the
    //  value type for the hash table, mapping tokenizer type to a specific function.
    using namespace std::placeholders;
    _tokenizer_types[Subtree]      = bind(&tree_tokenizer::subtreeTokenize,      this, _1, _2, _3);
    _tokenizer_types[Tag]          = bind(&tree_tokenizer::tagTokenize,          this, _1, _2, _3);
    _tokenizer_types[Branch]       = bind(&tree_tokenizer::branchTokenize,       this, _1, _2, _3);
    _tokenizer_types[Depth]        = bind(&tree_tokenizer::depthTokenize,        this, _1, _2, _3);
    _tokenizer_types[Skeleton]     = bind(&tree_tokenizer::skeletonTokenize,     this, _1, _2, _3);
    _tokenizer_types[SemiSkeleton] = bind(&tree_tokenizer::semiSkeletonTokenize, this, _1, _2, _3);
    _tokenizer_types[Multi]        = bind(&tree_tokenizer::multiTokenize,        this, _1, _2, _3);
}

void tree_tokenizer::tokenize(Document & document,
        const shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    vector<ParseTree> trees = ParseTree::getTrees(document.getPath() + ".tree");
    TokenizerFunction tFunc = _tokenizer_types[_type];
    for(auto & tree: trees)
        tFunc(document, tree, docFreq);
}

void tree_tokenizer::multiTokenize(Document & document, const ParseTree & tree,
        const shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    subtreeTokenize(document, tree, docFreq);
    tagTokenize(document, tree, docFreq);
    depthTokenize(document, tree, docFreq);
    branchTokenize(document, tree, docFreq);
}

void tree_tokenizer::subtreeTokenize(Document & document, const ParseTree & tree,
        const shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    string representation = tree.getChildrenString() + "|" + tree.getPOS();
    document.increment(mapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        subtreeTokenize(document, child, docFreq);
}

void tree_tokenizer::branchTokenize(Document & document, const ParseTree & tree,
        const shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    string representation = common::toString(tree.numChildren());
    document.increment(mapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        branchTokenize(document, child, docFreq);
}

void tree_tokenizer::tagTokenize(Document & document, const ParseTree & tree,
        const shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    string representation = tree.getPOS();
    document.increment(mapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        tagTokenize(document, child, docFreq);
}

void tree_tokenizer::depthTokenize(Document & document, const ParseTree & tree,
        const shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    size_t h = ParseTree::height(tree);
    string representation = common::toString(h);
    document.increment(mapping(representation), 1, docFreq);
}

void tree_tokenizer::skeletonTokenize(Document & document, const ParseTree & tree,
        const shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    string representation = tree.getSkeleton();
    document.increment(mapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        skeletonTokenize(document, child, docFreq);
}

void tree_tokenizer::semiSkeletonTokenize(Document & document, const ParseTree & tree,
        const shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    string representation = tree.getPOS() + tree.getSkeleton();
    document.increment(mapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        semiSkeletonTokenize(document, child, docFreq);
}

}
}
