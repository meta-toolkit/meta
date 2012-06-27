/**
 * Run shuffle.rb first to generating the testing and training lists for
 *  a given collection.
 */

#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <map>

#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/pos_tree_tokenizer.h"
#include "tokenizers/parse_tree.h"
#include "io/parser.h"
#include "index/ram_index.h"
#include "index/document.h"

using std::pair;
using std::vector;
using std::cout;
using std::endl;
using std::string;

inline string makeGreen(string str){ return "\033[1;32m" + str + "\033[0m"; }
inline string makeRed(string str){ return "\033[1;31m" + str + "\033[0m"; }

vector<string> getFilenames(const string & filename, const string & prefix)
{
    vector<string> files;
    Parser parser(filename, "\n");
    while(parser.hasNext())
        files.push_back(prefix + parser.next());
    return files;
}

int main(int argc, char* argv[])
{
    //string prefix = "/home/sean/projects/senior-thesis-data/20newsgroups/";
    //string prefix = "/home/sean/projects/senior-thesis-data/6reviewers/";
    string prefix = "/home/sean/projects/senior-thesis-data/10authors/";

    vector<string> trainFiles = getFilenames(prefix + "train.txt", prefix);
    vector<string> testFiles = getFilenames(prefix + "test.txt", prefix);

    Tokenizer* tokenizer = new NgramTokenizer(2);
    RAMIndex index(trainFiles, tokenizer);

    cout << "Running queries..." << endl;
    size_t numQueries = 1;
    size_t numCorrect = 0;
    for(vector<string>::iterator iter = testFiles.begin(); iter != testFiles.end(); ++iter)
    {
        string category = RAMIndex::getCategory(*iter);
        Document query(RAMIndex::getName(*iter), category);
        tokenizer->tokenize(*iter, query, NULL);
        string result = index.classifyKNN(query, 1);
        if(result == ( "(" + category + ")"))
        {
            ++numCorrect;
            cout << "  -> " << makeGreen("OK");
        }
        else
            cout << "  -> " << makeRed("incorrect");
        cout << " " << result << endl << "  -> " << ((double) numCorrect / numQueries * 100)
             << "% accuracy, " << numQueries << "/" << testFiles.size() << " processed " << endl;
        ++numQueries;
    }

    delete tokenizer;
    return 0;
}
