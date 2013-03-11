/**
 * @file search.cpp
 */

#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "io/config_reader.h"
#include "tokenizers/tokenizer.h"
#include "index/document.h"
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

    Tokenizer* tokenizer = ConfigReader::create_tokenizer(config);

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
