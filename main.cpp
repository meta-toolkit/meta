/**
 * @file main.cpp
 * 
 * Creates an index and runs queries on it.
 *
 * Run shuffle.rb first to generate the testing and training lists for
 *  a given collection.
 */

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <map>

#include "classify/confusion_matrix.h"
#include "io/config_reader.h"
#include "classify/knn.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "io/parser.h"
#include "index/ram_index.h"
#include "index/document.h"
#include "util/common.h"

using std::shared_ptr;
using std::pair;
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
 * @return whether the int representation of two strings is within k of each other
 */
bool withinK(const string & one, const string & two, int k)
{
    int valOne;
    int valTwo;
    istringstream(one) >> valOne;
    istringstream(two) >> valTwo;
    return abs(valTwo - valOne) <= k;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    unordered_map<string, string> config = ConfigReader::read(argv[1]);

    bool matrix = config["ConfusionMatrix"] == "yes";
    bool quiet = config["quiet"] == "yes";
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];

    std::shared_ptr<Tokenizer> tokenizer;
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
    
    vector<Document> documents = getDocs(prefix + "/full-corpus.txt", prefix);

    string method = config["method"];
    if(method == "ngram")
    {
        cout << "Running ngram tokenizer with n = " << nVal
             << " and submethod " << config["ngramOpt"] << endl;
        shared_ptr<Tokenizer> tok(new NgramTokenizer(nVal, ngramOpt[config["ngramOpt"]]));
        tokenizer = tok;
    }
    else if(method == "tree")
    {
        cout << "Running tree tokenizer with submethod " << config["treeOpt"] << endl;
        shared_ptr<Tokenizer> tok(new TreeTokenizer(treeOpt[config["treeOpt"]]));
        tokenizer = tok;
    }
    else
    {
        cerr << "Method was not able to be determined" << endl;
        return 1;
    }

    vector<Document> trainDocs = getDocs(prefix + "/train.txt", prefix);
    vector<Document> testDocs = getDocs(prefix + "/test.txt", prefix);
    shared_ptr<Index> index(new RAMIndex(trainDocs, tokenizer));

    size_t numQueries = 0;
    size_t numCorrect = 0;
    ConfusionMatrix confusionMatrix;

    for(auto & query: testDocs)
    {
        ++numQueries;
        string result = KNN::classify(query, index, kVal);
        if(matrix) confusionMatrix.add(result, query.getCategory());
        //if(withinK(result, query.getCategory(), 1))
        if(result == query.getCategory())
        {
            ++numCorrect;
            if(!quiet) cout << "  -> " << Common::makeGreen("OK");
        }
        else
            if(!quiet) cout << "  -> " << Common::makeRed("incorrect");
        if(!quiet) cout << " (" << result << ")" << endl << "  -> " << ((double) numCorrect / numQueries * 100)
             << "% accuracy, " << numQueries << "/" << testDocs.size() << " processed " << endl;
    }

    if(matrix) confusionMatrix.print();
    cout << "Trained on " << trainDocs.size() << " documents" << endl;
    cout << "Tested on " << testDocs.size() << " documents" << endl;
    cout << "Total accuracy: " << ((double) numCorrect / numQueries * 100) << endl;

    return 0;
}
