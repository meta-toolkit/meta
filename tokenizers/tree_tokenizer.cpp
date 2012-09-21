/**
 * @file tree_tokenizer.cpp
 */

#include <iostream>
#include <utility>
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
    string representation = tree.getChildrenString() + "|" + tree.getPOS();
    document.increment(getMapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        branchTokenize(document, child, docFreq);
}

void TreeTokenizer::tagTokenize(Document & document, const ParseTree & tree,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
}
void TreeTokenizer::depthTokenize(Document & document, const ParseTree & tree,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
}
