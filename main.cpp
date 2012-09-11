/**
 * @file main.cpp
 * 
 * Creates an index and runs queries on it.
 *
 * Run shuffle.rb first to generating the testing and training lists for
 *  a given collection.
 */

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <map>

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

int main(int argc, char* argv[])
{
    //string prefix = "/home/sean/projects/senior-thesis-data/20newsgroups/";
    //string prefix = "/home/sean/projects/senior-thesis-data/6reviewers/";
    string prefix = "/home/sean/projects/senior-thesis-data/10authors/";

    vector<Document> trainDocs = getDocs(prefix + "train.txt", prefix);
    vector<Document> testDocs = getDocs(prefix + "test.txt", prefix);

    std::shared_ptr<Tokenizer> tokenizer(new FWTokenizer("data/function-words.txt"));
    std::unique_ptr<Index> index(new RAMIndex(trainDocs, tokenizer));

    cout << "Running queries..." << endl;
    size_t numQueries = 1;
    size_t numCorrect = 0;
    for(auto & query: testDocs)
    {
        tokenizer->tokenize(query, NULL);
        string result = index->classifyKNN(query, 1);
        if(result == ( "(" + query.getCategory() + ")"))
        {
            ++numCorrect;
            cout << "  -> " << Common::makeGreen("OK");
        }
        else
            cout << "  -> " << Common::makeRed("incorrect");
        cout << " " << result << endl << "  -> " << ((double) numCorrect / numQueries * 100)
             << "% accuracy, " << numQueries << "/" << testDocs.size() << " processed " << endl;
        ++numQueries;
    }

    return 0;
}
