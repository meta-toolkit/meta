/**
 * @file liblinear.cpp
 * This creates input for liblinear based on features extracted from my
 *  tokenizers.
 */

#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/fw_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "io/parser.h"
#include "util/common.h"

using std::shared_ptr;
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
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];
    string method = config["method"];

    int nVal;
    istringstream(config["ngram"]) >> nVal;

    unordered_map<string, NgramTokenizer::NgramType> ngramOpt = {
        {"POS", NgramTokenizer::POS}, {"Word", NgramTokenizer::Word}
    };

    unordered_map<string, TreeTokenizer::TreeTokenizerType> treeOpt = {
        {"Subtree", TreeTokenizer::Subtree}, {"Depth", TreeTokenizer::Depth},
        {"Branch", TreeTokenizer::Branch}, {"Tag", TreeTokenizer::Tag}
    };
    
    vector<Document> documents = getDocs(prefix + "/full-corpus.txt", prefix);

    if(method == "ngram")
    {
        //cerr << "Running ngram tokenizer with n = " << nVal
        //     << " and submethod " << config["ngramOpt"] << endl;
        shared_ptr<Tokenizer> tokenizer(new NgramTokenizer(nVal, ngramOpt[config["ngramOpt"]]));
        for(auto & doc: documents)
        {
            tokenizer->tokenize(doc, NULL);
            doc.printLiblinearData();
        }
    }
    else if(method == "tree")
    {
        //cerr << "Running tree tokenizer with submethod " << config["treeOpt"] << endl;
        shared_ptr<Tokenizer> tokenizer(new TreeTokenizer(treeOpt[config["treeOpt"]]));
        for(auto & doc: documents)
        {
            tokenizer->tokenize(doc, NULL);
            doc.printLiblinearData();
        }
    }
    else
        cerr << "Method was not able to be determined" << endl;
}
