/**
 * @file classify-test.cpp
 */

#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "util/invertible_map.h"
#include "classify/naive_bayes.h"

using std::vector;
using std::unordered_map;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace meta;
using namespace meta::index;
using namespace meta::util;
using namespace meta::tokenizers;

/**
 * Tokenizes testing and training docs.
 */
void tokenize(vector<Document> & train_docs, vector<Document> & test_docs, const unordered_map<string, string> & config)
{
    std::shared_ptr<tokenizer> tok = io::config_reader::create_tokenizer(config);

    size_t i = 0;
    for(auto & d: train_docs)
    {
        common::show_progress(i++, train_docs.size(), 20, "  tokenizing training docs ");
        tok->tokenize(d, nullptr);
    }
    common::end_progress("  tokenizing training docs ");

    i = 0;
    for(auto & d: test_docs)
    {
        common::show_progress(i++, test_docs.size(), 20, "  tokenizing testing docs ");
        tok->tokenize(d, nullptr);
    }
    common::end_progress("  tokenizing testing docs ");
}

/**
 *
 */
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.ini" << endl;
        return 1;
    }

    unordered_map<string, string> config = io::config_reader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];
    vector<Document> train_docs = Document::loadDocs(prefix + "/train.txt", prefix);
    vector<Document> test_docs = Document::loadDocs(prefix + "/test.txt", prefix);

    tokenize(train_docs, test_docs, config);
   
    classify::naive_bayes nb;
    nb.train(train_docs);
    classify::ConfusionMatrix matrix = nb.test(test_docs);
    matrix.print();
    matrix.print_stats();

    return 0;
}
