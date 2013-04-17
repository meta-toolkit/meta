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
#include "classify/liblinear_svm.h"
#include "classify/perceptron.h"

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
void tokenize(vector<Document> & docs, const unordered_map<string, string> & config)
{
    std::shared_ptr<tokenizer> tok = io::config_reader::create_tokenizer(config);

    size_t i = 0;
    for(auto & d: docs)
    {
        common::show_progress(i++, docs.size(), 20, "  tokenizing ");
        tok->tokenize(d, nullptr);
    }
    common::end_progress("  tokenizing ");
}

void cv( classify::classifier & c, const vector<Document> & train_docs ) {
    classify::confusion_matrix matrix = c.cross_validate(train_docs, 5);
    matrix.print();
    matrix.print_stats();
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
    string prefix = config["prefix"] + config["dataset"];
    vector<Document> train_docs = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    //vector<Document> test_docs = Document::loadDocs(prefix + "/test.txt", prefix);

    tokenize(train_docs, config);
    //tokenize(test_docs, config);
    
    classify::liblinear_svm svm(config["liblinear"]);
    classify::perceptron p;
    classify::naive_bayes nb;
    cv( svm, train_docs );
    cv( p, train_docs );
    cv( nb, train_docs );
    //svm.train(train_docs);
    
    return 0;
}
