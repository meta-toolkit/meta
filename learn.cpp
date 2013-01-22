/**
 * @file learn.cpp
 * This creates input for liblinear based on features extracted from my
 *  tokenizers.
 */

#include <omp.h>
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
using std::endl;
using std::string;

/**
 * Returns a vector of all documents in a given dataset.
 * @param filename - the file containing the list of files in a corpus
 * @param prefix - the prefix of the path to a corpus
 * @return a vector of Documents created from the filenames
 */
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

    if(config["parallel"] == "no")
        omp_set_num_threads(1);

    int nVal;
    istringstream(config["ngram"]) >> nVal;

    unordered_map<string, NgramTokenizer::NgramType> ngramOpt = {
        {"POS", NgramTokenizer::POS}, {"Word", NgramTokenizer::Word},
        {"FW", NgramTokenizer::FW}
    };

    unordered_map<string, TreeTokenizer::TreeTokenizerType> treeOpt = {
        {"Subtree", TreeTokenizer::Subtree}, {"Depth", TreeTokenizer::Depth},
        {"Branch", TreeTokenizer::Branch}, {"Tag", TreeTokenizer::Tag},
        {"Skeleton", TreeTokenizer::Skeleton}, {"SemiSkeleton", TreeTokenizer::SemiSkeleton},
        {"Multi", TreeTokenizer::Multi}
    };
    
    vector<Document> documents = getDocs(prefix + "/full-corpus.txt", prefix);

    size_t done = 0;
    if(method == "ngram")
    {
        Tokenizer* tokenizer = new NgramTokenizer(nVal, ngramOpt[config["ngramOpt"]]);
        #pragma omp parallel for
        for(size_t i = 0; i < documents.size(); ++i)
        {
            tokenizer->tokenize(documents[i], NULL);
            #pragma omp critical
            {
                cout << documents[i].getLiblinearData(mapping);
                if(!quiet && done++ % 10 == 0)
                    cerr << "  Tokenizing " << static_cast<double>(done) / documents.size() * 100 << "%     \r"; 
            }
        }
        delete tokenizer;
    }
    else if(method == "tree")
    {
        #pragma omp parallel for
        for(size_t i = 0; i < documents.size(); ++i)
        {
            Tokenizer* tokenizer = new TreeTokenizer(treeOpt[config["treeOpt"]]);
            tokenizer->tokenize(documents[i], NULL);
            #pragma omp critical
            {
                cout << documents[i].getLiblinearData(mapping);
                if(!quiet && done++ % 10 == 0)
                    cerr << "  Tokenizing " << static_cast<double>(done) / documents.size() * 100 << "%     \r"; 
            }
            delete tokenizer;
        }
    }
    else if(method == "both")
    {
        Tokenizer* treeTokenizer = new TreeTokenizer(treeOpt[config["treeOpt"]]);
        Tokenizer* ngramTokenizer = new NgramTokenizer(nVal, ngramOpt[config["ngramOpt"]]);
        #pragma omp parallel for
        for(size_t i = 0; i < documents.size(); ++i)
        {
            treeTokenizer->tokenize(documents[i], NULL);
            ngramTokenizer->tokenize(documents[i], NULL);
            #pragma omp critical
            {
                cout << documents[i].getLiblinearData(mapping);
                if(!quiet && done++ % 10 == 0)
                    cerr << "  Tokenizing " << static_cast<double>(done) / documents.size() * 100 << "%     \r"; 
            }
        }
        delete treeTokenizer;
        delete ngramTokenizer;
    }
    else
        cerr << "Method was not able to be determined" << endl;

    if(!quiet)
        cerr << "  Tokenizing 100%         " << endl;
}
