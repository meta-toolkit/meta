#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "util/invertible_map.h"
#include "classify/feature_select/all.h"
#include "classify/confusion_matrix.h"
#include "classify/classifier/liblinear_svm.h"
#include "topics/slda.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ofstream;

using namespace meta;
using namespace meta::index;
using namespace meta::util;
using namespace meta::tokenizers;

/**
 * Cross-validate documents with a specific classifier.
 */
classify::confusion_matrix cv(classify::classifier & c, const vector<document> & train_docs,
        const classify::confusion_matrix & orig)
{
    classify::confusion_matrix matrix = c.cross_validate(train_docs, 5);
    cout << matrix.accuracy();
    cout << ((classify::confusion_matrix::mcnemar_significant(orig, matrix)) ? "* " : "  ");
    return matrix;
}

/**
 * Tokenize documents based on options set in config.
 */
void tokenize(vector<document> & docs, const cpptoml::toml_group & config)
{
    std::shared_ptr<tokenizer> tok = io::config_reader::create_tokenizer(config);

    size_t i = 0;
    for(auto & d: docs)
    {
        common::show_progress(i++, docs.size(), 20, "  tokenizing ");
        tok->tokenize(d);
    }
    common::end_progress("  tokenizing ");
}

/**
 * Return irrelevant features.
 */
vector<pair<term_id, double>> top_features(const vector<pair<term_id, double>> & orig, double percent)
{
    size_t size = percent * orig.size();
    return vector<pair<term_id, double>>(orig.begin(), orig.begin() + size);
}

/**
 * Run feature selection with varying amounts of features.
 */
void test(const vector<document> & docs, const vector<pair<term_id, double>> & features, classify::classifier & c,
        const classify::confusion_matrix & orig)
{
    for(auto percent: {.01, .05, .10, .15, .20, .25})
    {
        auto selected_features = top_features(features, percent);
        vector<document> reduced = document::filter_features(docs, selected_features);
        cv(c, reduced, orig);
    }
    cout << endl;
}

/**
 * Compare various feature selection algorithms:
 *  - Information gain,
 *  - Chi square,
 *  - sLDA.
 * Use these feature selection metrics with SVM and Naive Bayes.
 * Use the following single features:
 *  - unigram and bigram words,
 *  - bigram POS tags,
 *  - rewrite rules,
 *  - annotated skeleton.
 * And the following combined features:
 *  - unigram words + bigram words,
 *  - unigram words + bigram POS tags,
 *  - unigram words + bigram words + bigram POS tags,
 *  - unigram words + annotated skeleton,
 *  - unigram words + rewrite rules,
 *  - unigram words + bigram words + annotated skeleton,
 *  - unigram words + bigram words + rewrite rules.
 * With the following data sets:
 *  - sentiment, nationality, 
 */
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.ini" << endl;
        return 1;
    }

    auto config = io::config_reader::read(argv[1]);
    string prefix = *cpptoml::get_as<std::string>( config, "prefix" )
        + *cpptoml::get_as<std::string>( config, "dataset" );
    string corpus_file = prefix 
        + "/" 
        + *cpptoml::get_as<std::string>( config, "list" )
        + "-full-corpus.txt";
    vector<document> docs = document::load_docs(corpus_file, prefix);
    tokenize(docs, config);
   
    // baseline
    classify::liblinear_svm svm{ *cpptoml::get_as<std::string>( config, "liblinear" ) };
    classify::confusion_matrix orig = svm.cross_validate(docs, 5);
    cout << "Original accuracy: " << orig.accuracy() << endl;

    // information gain
    cerr << endl << "Running information gain..." << endl;
    classify::info_gain ig(docs);
    test(docs, ig.select(), svm, orig);

    // chi square
    cerr << endl << "Running Chi square..." << endl;
    classify::chi_square cs(docs);
    test(docs, cs.select(), svm, orig);

    // sLDA
    cerr << endl << "Running sLDA..." << endl;
    topics::slda lda{ *cpptoml::get_as<std::string>( config, "slda" ), .1 };
    lda.estimate(docs);
    auto features = lda.select();
    test(docs, features, svm, orig);
    cout << features.size() << " total features" << endl;

    return 0;
}
