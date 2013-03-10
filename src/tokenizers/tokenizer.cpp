#include <fstream>
#include <ostream>
#include "util/invertible_map.h"
#include "index/document.h"
#include "tokenizers/parse_tree.h"
#include "tokenizers/tokenizer.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::unordered_map;

Tokenizer::Tokenizer():
    _currentTermID(0),
    _termMap(InvertibleMap<TermID, string>())
{ /* nothing */ }

TermID Tokenizer::getMapping(const string & term)
{
    if(!_termMap.containsValue(term))
    {
        _termMap.insert(_currentTermID, term);
        return _currentTermID++;
    }
    else
    {
        TermID termID = _termMap.getKeyByValue(term);
        return termID;
    }
}

void Tokenizer::setTermIDMapping(const InvertibleMap<TermID, string> & mapping)
{
    _termMap = mapping;
    _currentTermID = _termMap.size();
}

void Tokenizer::saveTermIDMapping(const string & filename) const
{
    _termMap.saveMap(filename);
}

InvertibleMap<TermID, std::string> Tokenizer::getTermIDMapping() const
{
    return _termMap;
}

string Tokenizer::getLabel(TermID termID) const
{
    return _termMap.getValueByKey(termID);
}

void Tokenizer::printData() const
{
    for(auto & term: _termMap)
        cout << term.first << "\t" << term.second << endl;
}

void Tokenizer::setMaxTermID(size_t start)
{
    _currentTermID = start;
}

size_t Tokenizer::getNumTerms() const
{
    return _termMap.size();

}

/*
 * This breaks encapsulation. Maybe create_from_config shouldn't be a member of
 * Tokenizer.
 */
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"

Tokenizer* Tokenizer::create_from_config(const unordered_map<string, string> & config)
{
    string method = config.at("method");
    
    if(method == "ngram")
    {
        unordered_map<string, NgramTokenizer::NgramType> ngramOpt = {
            {"POS", NgramTokenizer::POS}, {"Word", NgramTokenizer::Word},
            {"FW", NgramTokenizer::FW}, {"Char", NgramTokenizer::Char}
        };

        int nVal;
        istringstream(config.at("ngram")) >> nVal;
        return new NgramTokenizer(nVal, ngramOpt[config.at("ngramOpt")]);
    }
    else if(method == "tree")
    {
        unordered_map<string, TreeTokenizer::TreeTokenizerType> treeOpt = {
            {"Subtree", TreeTokenizer::Subtree}, {"Depth", TreeTokenizer::Depth},
            {"Branch", TreeTokenizer::Branch}, {"Tag", TreeTokenizer::Tag},
            {"Skel", TreeTokenizer::Skeleton}, {"Semi", TreeTokenizer::SemiSkeleton}
        };

        return new TreeTokenizer(treeOpt[config.at("treeOpt")]);
    }

    cerr << "Method was not able to be determined" << endl;
    return nullptr;
}
