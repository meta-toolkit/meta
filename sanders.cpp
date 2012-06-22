#include <iostream>
#include <fstream>
#include "io/parser.h"
#include "index/ram_index.h"
#include "tokenizers/sanders_tokenizer.h"

using std::ofstream;
using std::vector;
using std::cout;
using std::endl;
using std::string;

/**
 * makeGreen - color a string green
 * @param str - the string to color
 */
inline string makeGreen(string str)
{
    return "\033[1;32m" + str + "\033[0m";
}

/**
 * makeRed - color a string red
 * @param str - the string to color
 */
inline string makeRed(string str)
{
    return "\033[1;31m" + str + "\033[0m";
}

vector<Document> getDocs(const string & path, Tokenizer* tokenizer)
{
    vector<Document> docs;
    Parser parser(path, "\n");
    while(parser.hasNext())
    {
        string category = parser.next();
        string content = parser.next();
        Document doc(content, category);
        tokenizer->tokenize(content, doc, NULL);
        docs.push_back(doc);
    }
    return docs;
}

int main(int argc, char* argv[])
{
    Tokenizer* tokenizer = new SandersTokenizer(2);

    string prefix = "/home/sean/projects/senior-thesis-data/sanders/";
    vector<Document> trainFiles = getDocs(prefix + "train.txt", tokenizer);
    vector<Document> testFiles = getDocs(prefix + "test.txt", tokenizer);

    RAMIndex index(trainFiles, tokenizer);
    cout << "Running queries..." << endl;
    size_t numQueries = 1;
    size_t numCorrect = 0;
    for(vector<Document>::iterator query = testFiles.begin(); query != testFiles.end(); ++query)
    {
        size_t numResults = 0;
        string result = index.classifyKNN(*query, 1);
        size_t space = result.find_last_of(" ") + 2;
        if(result != "[no results]")
            result = result.substr(space, result.size() - space - 1);
        if(result == query->getCategory())
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
