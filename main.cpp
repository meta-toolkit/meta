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

#include "classify/knn.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/pos_tree_tokenizer.h"
#include "tokenizers/fw_tokenizer.h"
#include "tokenizers/parse_tree.h"
#include "io/parser.h"
#include "index/ram_index.h"
#include "index/document.h"
#include "util/common.h"

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
        docs.push_back(Document(prefix + file));
    }
    return docs;
}

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
    bool quiet = argc > 1;

    string prefix = "/home/sean/projects/senior-thesis-data/kaggle/";
    //string prefix = "/home/sean/projects/senior-thesis-data/20newsgroups/";
    //string prefix = "/home/sean/projects/senior-thesis-data/6reviewers/";
    //string prefix = "/home/sean/projects/senior-thesis-data/10authors/";

    vector<Document> trainDocs = getDocs(prefix + "train.txt", prefix);
    vector<Document> testDocs = getDocs(prefix + "test.txt", prefix);

    std::shared_ptr<Tokenizer> tokenizer(new NgramTokenizer(2));
    //std::shared_ptr<Tokenizer> tokenizer(new FWTokenizer("data/function-words.txt"));
    std::shared_ptr<Index> index(new RAMIndex(trainDocs, tokenizer));

    cout << "Running queries..." << endl;
    size_t numQueries = 1;
    size_t numCorrect = 0;
    for(auto & query: testDocs)
    {
        tokenizer->tokenize(query, NULL);
        string result = KNN::classify(query, index, 1);
        //if(result == query.getCategory())
        if(withinK(result, query.getCategory(), 0))
        {
            ++numCorrect;
            if(!quiet) cout << "  -> " << Common::makeGreen("OK");
        }
        else
            if(!quiet) cout << "  -> " << Common::makeRed("incorrect");
        if(!quiet) cout << " (" << result << ")" << endl << "  -> " << ((double) numCorrect / numQueries * 100)
             << "% accuracy, " << numQueries << "/" << testDocs.size() << " processed " << endl;
        ++numQueries;
    }

    cout << "Trained on " << trainDocs.size() << " documents" << endl;
    cout << "Tested on " << testDocs.size() << " documents" << endl;
    cout << "Total accuracy: " << ((double) numCorrect / numQueries * 100) << endl;

    return 0;
}
