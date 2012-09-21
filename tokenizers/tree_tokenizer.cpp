/**
 * @file tree_tokenizer.cpp
 */

#include <iostream>
#include <utility>
#include "util/common.h"
#include "index/document.h"
#include "parse_tree.h"
#include "tree_tokenizer.h"

using std::vector;
using std::unordered_map;
using std::pair;
using std::bind;

const char* TreeTokenizer::_extension = ".tree";

TreeTokenizer::TreeTokenizer(TreeTokenizerType type):
    _type(type),
    _tokenizerTypes(unordered_map<TreeTokenizerType, TokenizerFunction, std::hash<int>>())
{
    /**
     * Bind the member tokenizer functions to the current object, and use them as the
     *  value type for the hash table, mapping tokenizer type to a specific function.
     */
    using namespace std::placeholders;
    _tokenizerTypes[Subtree] = bind(&TreeTokenizer::subtreeTokenize, this, _1, _2, _3);
    _tokenizerTypes[Tag]     = bind(&TreeTokenizer::tagTokenize,     this, _1, _2, _3);
    _tokenizerTypes[Branch]  = bind(&TreeTokenizer::branchTokenize,  this, _1, _2, _3);
    _tokenizerTypes[Depth]   = bind(&TreeTokenizer::depthTokenize,   this, _1, _2, _3);
}

void TreeTokenizer::tokenize(Document & document, std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    vector<ParseTree> trees = ParseTree::getTrees(document.getPath() + _extension);
    TokenizerFunction tFunc = _tokenizerTypes[_type];
    for(auto & tree: trees)
        tFunc(document, tree, docFreq);
}

void TreeTokenizer::subtreeTokenize(Document & document, const ParseTree & tree,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    string representation = tree.getChildrenString() + "|" + tree.getPOS();
    document.increment(getMapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        subtreeTokenize(document, child, docFreq);
}

void TreeTokenizer::branchTokenize(Document & document, const ParseTree & tree,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    string representation = Common::toString(tree.numChildren());
    document.increment(getMapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        branchTokenize(document, child, docFreq);
}

void TreeTokenizer::tagTokenize(Document & document, const ParseTree & tree,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    /**
     * Getting some random output here. Is getPOS/parsetree working?
     */
    string representation = tree.getPOS();
    //std::cout << "tag: " << representation << std::endl;
    document.increment(getMapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        tagTokenize(document, child, docFreq);
}

void TreeTokenizer::depthTokenize(Document & document, const ParseTree & tree,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    /**
     * Surprisingly, it seems like tree depth is not correlated with
     *  standardized test essay scores.
     */
    size_t h = ParseTree::height(tree);
    //std::cout << ": " << h << std::endl;
    string representation = Common::toString(h);
    document.increment(getMapping(representation), 1, docFreq);
}
