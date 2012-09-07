/**
 * @file fw_tokenizer.cpp
 */

#include "io/parser.h"
#include "index/document.h"
#include "fw_tokenizer.h"

using std::unordered_set;
using std::unordered_map;
using std::string;

FWTokenizer::FWTokenizer(const string & fwFile)
{
    Parser parser(fwFile, " \n");
    while(parser.hasNext())
        _functionWords.insert(parser.next());
}

void FWTokenizer::tokenize(Document & document, unordered_map<TermID, unsigned int>* docFreq)
{
    string validchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'-";
    Parser parser(document.getPath(), validchars, validchars, validchars);
    while(parser.hasNext())
    {
        string token = parser.next();
        if(_functionWords.find(token) != _functionWords.end())
            document.increment(getMapping(token), 1, docFreq);
    }
}
