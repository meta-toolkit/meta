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
    string prefix = config["prefix"];

    vector<Document> trainDocs = getDocs(prefix + "/train.txt", prefix);
    vector<Document> testDocs = getDocs(prefix + "/test.txt", prefix);

    //std::shared_ptr<Tokenizer> wordTokenizer(new NgramTokenizer(1, NgramTokenizer::Word));
    std::shared_ptr<Tokenizer> posTokenizer(new NgramTokenizer(5, NgramTokenizer::POS));
    //std::shared_ptr<Index> wordIndex(new RAMIndex(trainDocs, wordTokenizer));
    std::shared_ptr<Index> posIndex(new RAMIndex(trainDocs, posTokenizer));

    //std::shared_ptr<Tokenizer> treeTokenizer(new TreeTokenizer(TreeTokenizer::Subtree));
    //std::shared_ptr<Index> treeIndex(new RAMIndex(trainDocs, treeTokenizer));

    cout << "Running queries..." << endl;
    size_t numQueries = 0;
    size_t numCorrect = 0;
    ConfusionMatrix confusionMatrix;

    for(auto & query: testDocs)
    {
        ++numQueries;
        string result = KNN::classify(query, posIndex, 1);
        //string result = KNN::classify(query, {wordIndex, posIndex}, {0.5, 0.5}, 1);
        //if(withinK(result, query.getCategory(), 1))
        if(matrix) confusionMatrix.add(result, query.getCategory());
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
