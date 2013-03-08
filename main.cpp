/**
 * @file main.cpp
 */

#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "classify/confusion_matrix.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "index/document.h"
#include "util/common.h"
#include "cluster/similarity.h"

using std::pair;
using std::vector;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    unordered_map<string, string> config = ConfigReader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];

    Tokenizer* tokenizer;
    int nVal;
    int kVal;
    istringstream(config["ngram"]) >> nVal;
    istringstream(config["knn"]) >> kVal;

    unordered_map<string, NgramTokenizer::NgramType> ngramOpt = {
        {"POS", NgramTokenizer::POS}, {"Word", NgramTokenizer::Word},
        {"FW", NgramTokenizer::FW}
    };

    unordered_map<string, TreeTokenizer::TreeTokenizerType> treeOpt = {
        {"Subtree", TreeTokenizer::Subtree}, {"Depth", TreeTokenizer::Depth},
        {"Branch", TreeTokenizer::Branch}, {"Tag", TreeTokenizer::Tag}
    };
    
    string method = config["method"];
    if(method == "ngram")
        tokenizer = new NgramTokenizer(nVal, ngramOpt[config["ngramOpt"]]);
    else if(method == "tree")
        tokenizer = new TreeTokenizer(treeOpt[config["treeOpt"]]);
    else
    {
        cerr << "Method was not able to be determined" << endl;
        return 1;
    }

    vector<Document> docs = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    for(auto & query: docs)
    {
        tokenizer->tokenize(query);
        cout << "cosine similarity:  " << Document::cosine_similarity(docs[0], query) << endl;
        cout << "jaccard similarity: " << Document::jaccard_similarity(docs[0], query) << endl;
        cout << endl;
    }

    delete tokenizer;
    return 0;
}
