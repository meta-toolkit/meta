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
#include "tokenizers/fw_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
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

    //string prefix = "/home/sean/projects/senior-thesis-data/kaggle/";
    //string prefix = "/home/sean/projects/senior-thesis-data/20newsgroups/";
    string prefix = "/home/sean/projects/senior-thesis-data/6reviewers/";
    //string prefix = "/home/sean/projects/senior-thesis-data/10authors/";

    vector<Document> trainDocs = getDocs(prefix + "train.txt", prefix);
    vector<Document> testDocs = getDocs(prefix + "test.txt", prefix);

    //std::shared_ptr<Tokenizer> wordTokenizer(new NgramTokenizer(2, NgramTokenizer::Word));
    //std::shared_ptr<Tokenizer> posTokenizer(new NgramTokenizer(6, NgramTokenizer::POS));
    //std::shared_ptr<Index> wordIndex(new RAMIndex(trainDocs, wordTokenizer));
    //std::shared_ptr<Index> posIndex(new RAMIndex(trainDocs, posTokenizer));

    std::shared_ptr<Tokenizer> treeTokenizer(new TreeTokenizer(TreeTokenizer::SubtreeCounter));
    //std::shared_ptr<Tokenizer> treeTokenizer(new TreeTokenizer(TreeTokenizer::ConditionalChildren));
    std::shared_ptr<Index> treeIndex(new RAMIndex(trainDocs, treeTokenizer));

    cout << "Running queries..." << endl;
    size_t numQueries = 0;
    size_t numCorrect = 0;

    for(auto & query: testDocs)
    {
        ++numQueries;
        string result = KNN::classify(query, treeIndex, 1);
        //string result = KNN::classify(query, {wordIndex, posIndex}, {0.5, 0.5}, 1);
        //if(withinK(result, query.getCategory(), 0))
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

    cout << "Trained on " << trainDocs.size() << " documents" << endl;
    cout << "Tested on " << testDocs.size() << " documents" << endl;
    cout << "Total accuracy: " << ((double) numCorrect / numQueries * 100) << endl;

    return 0;
}
