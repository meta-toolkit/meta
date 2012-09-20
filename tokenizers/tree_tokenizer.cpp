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

const char* TreeTokenizer::_extension = ".tree";

TreeTokenizer::TreeTokenizer(TreeTokenizerType type):
    _type(type),
    _tokenizerTypes(unordered_map<TreeTokenizerType, TokenizerFunction, std::hash<int>>())
{
    //_tokenizerTypes.insert(pair<TreeTokenizerType, TokenizerFunction>(ConditionalChildren, condChildrenTokenize));
    //_tokenizerTypes[ConditionalChildren] = condChildrenTokenize;
    //_tokenizerTypes.insert({SubtreeCounter, TreeTokenizer::subtreeCountTokenize});
}

void TreeTokenizer::tokenize(Document & document, std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    vector<ParseTree> trees = ParseTree::getTrees(document.getPath() + _extension);
    for(auto & tree: trees)
        _tokenizerTypes[_type](document, tree, docFreq);

    /*
    switch(_type)
    {
        case ConditionalChildren:
            for(auto & tree: trees)
                condChildrenTokenize(document, tree, docFreq);
            break;
        case SubtreeCount:
            for(auto & tree: trees)
                subtreeCountTokenize(document, tree, docFreq);
            break;
        default:
            std::cerr << "[TreeTokenizer]: tokenize: invalid TreeTokenizerType" << std::endl;
    }
    */
}

void TreeTokenizer::condChildrenTokenize(Document & document, const ParseTree & tree,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    string representation = tree.getChildrenString() + "|" + tree.getPOS();
    //std::cout << representation << std::endl;
    document.increment(getMapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        condChildrenTokenize(document, child, docFreq);
}

void TreeTokenizer::subtreeCountTokenize(Document & document, const ParseTree & tree,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    string representation = tree.getChildrenString() + "|" + tree.getPOS();
    //std::cout << representation << std::endl;
    document.increment(getMapping(representation), 1, docFreq);
    for(auto & child: tree.getChildren())
        condChildrenTokenize(document, child, docFreq);
}
