/**
 * @file learn.cpp
 * This creates input for liblinear based on features extracted from my
 *  tokenizers.
 */

#include <vector>
#include <string>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "io/parser.h"
#include "util/common.h"
#include "util/invertible_map.h"

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::unordered_map;

/**
 * Runs the scatterplot creation.
 */
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    unordered_map<string, string> config = ConfigReader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];
    string method = config["method"];
    bool quiet = (config["quiet"] == "yes");
    InvertibleMap<string, int> mapping; // for unique ids when printing liblinear data

    vector<Document> documents = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    Tokenizer* tokenizer = ConfigReader::create_tokenizer(config);

    for(size_t i = 0; i < documents.size(); ++i)
    {
        // order of lines in the liblinear input file does NOT matter (tested)
        tokenizer->tokenize(documents[i], nullptr);
        cout << documents[i].getLearningData(mapping, false /* using liblinear */);
        if(!quiet && i % 20 == 0)
            cerr << "  tokenizing " << static_cast<double>(i) / documents.size() * 100 << "%     \r"; 
    }

    tokenizer->saveTermIDMapping("termidmapping.txt");
    delete tokenizer;
    
    if(!quiet)
        cerr << "\r";
}
