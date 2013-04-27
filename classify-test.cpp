/**
 * @file classify-test.cpp
 */

#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/tokenizers.h"
#include "util/invertible_map.h"
#include "classify/classifier/all.h"

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
void tokenize(vector<document> & docs, const unordered_map<string, string> & config)
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

classify::confusion_matrix cv( classify::classifier & c, const vector<document> & train_docs ) {
    classify::confusion_matrix matrix = c.cross_validate(train_docs, 5);
    matrix.print();
    matrix.print_stats();
    return matrix;
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
    string corpus_file = prefix + "/" + config["list"] + "-full-corpus.txt";
    vector<document> train_docs = document::load_docs(corpus_file, prefix);
    //vector<document> test_docs = document::loadDocs(prefix + "/test.txt", prefix);

    tokenize(train_docs, config);
    //tokenize(test_docs, config);
    
    classify::liblinear_svm svm(config["liblinear"]);
    classify::linear_svm l2svm;
    classify::linear_svm l1svm{ classify::linear_svm::loss_function::L1 };
    classify::perceptron p;
    classify::naive_bayes nb;
    auto m1 = cv( svm, train_docs );
    auto m2 = cv( l2svm, train_docs );
    std::cout << "(liblinear) Significant? " << std::boolalpha << classify::confusion_matrix::mcnemar_significant( m1, m2 ) << std::endl;
    auto m3 = cv( l1svm, train_docs );
    std::cout << "(liblinear) Significant? " << std::boolalpha << classify::confusion_matrix::mcnemar_significant( m1, m3 ) << std::endl;
    auto m4 = cv( p, train_docs );
    std::cout << "(liblinear) Significant? " << std::boolalpha << classify::confusion_matrix::mcnemar_significant( m1, m4 ) << std::endl;
    std::cout << "(lsvm) Significant? " << std::boolalpha << classify::confusion_matrix::mcnemar_significant( m2, m4 ) << std::endl;
    cv( nb, train_docs );
    //svm.train(train_docs);
    
    return 0;
}
