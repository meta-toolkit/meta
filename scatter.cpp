/**
 * @file scatter.cpp
 *
 * Creates histograms of some features to see if they will be useful.
 */

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <map>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "io/parser.h"
#include "util/common.h"

using std::unordered_map;
using std::pair;
using std::vector;
using std::cout;
using std::endl;
using std::string;

vector<Document> getDocs(const string & filename, const string & prefix)
{
    vector<Document> docs;
    Parser parser(filename, "\n");
    while(parser.hasNext())
    {
        string file = parser.next();
        docs.push_back(Document(prefix + "/" + file));
    }
    return docs;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    unordered_map<string, string> config = ConfigReader::read(argv[1]);

    bool quiet = config["quiet"] == "yes";
    string prefix = config["prefix"];

    vector<Document> docs = getDocs(prefix + "/" + config["language"] + ".txt", prefix);
    std::shared_ptr<Tokenizer> tokenizer(new TreeTokenizer(TreeTokenizer::Tag));

    std::shared_ptr<unordered_map<TermID, unsigned int>> collection(new unordered_map<TermID, unsigned int>());
    for(auto & doc: docs)
        tokenizer->tokenize(doc, collection);

    unsigned int totalTokens = 0;
    for(auto & freq: *collection)
        totalTokens += freq.second;

    for(auto & freq: *collection)
        cout << tokenizer->getLabel(freq.first)
             << " " << (double) freq.second / totalTokens << endl;

    return 0;
}
