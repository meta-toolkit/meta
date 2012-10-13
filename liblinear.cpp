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
    string prefix = config["prefix"];
    vector<Document> documents = getDocs(prefix + "/full-corpus.txt", prefix);

    //std::shared_ptr<Tokenizer> tokenizer(new NgramTokenizer(2, NgramTokenizer::Word));
    //std::shared_ptr<Tokenizer> tokenizer(new NgramTokenizer(5, NgramTokenizer::POS));
    std::shared_ptr<Tokenizer> tokenizer(new TreeTokenizer(TreeTokenizer::Subtree));

    for(auto & doc: documents)
    {
        tokenizer->tokenize(doc, NULL);
        doc.printLiblinearData();
    }
}
