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
#include "classify/select_chi_square.h"
#include "classify/select_info_gain.h"
#include "classify/confusion_matrix.h"
#include "classify/liblinear_svm.h"
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
classify::confusion_matrix cv( classify::classifier & c, const vector<Document> & train_docs )
{
    classify::confusion_matrix matrix = c.cross_validate(train_docs, 5);
    matrix.print();
    matrix.print_stats();
    return matrix;
}

/**
 * Tokenize documents based on options set in config.
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

/**
 * Return irrelevant features.
 */
vector<pair<term_id, double>> top_features(const vector<pair<term_id, double>> & orig, double percent)
{
    size_t start = percent * orig.size();
    return vector<pair<term_id, double>>(orig.begin() + start, orig.end());
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

    unordered_map<string, string> config = io::config_reader::read(argv[1]);
    string prefix = config["prefix"] + config["dataset"];
    vector<Document> docs = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    tokenize(docs, config);
   
    // baseline
    classify::liblinear_svm svm(config["liblinear"]);
    classify::confusion_matrix orig_matrix = cv(svm, docs);

    // information gain
    cout << "Running information gain..." << endl;
    classify::select_info_gain ig(docs);
    auto ig_features = top_features(ig.select(), .1);
    vector<Document> reduced = Document::filter_features(docs, ig_features);
    cv(svm, reduced);

    // chi square
    cout << "Running Chi square..." << endl;
    classify::select_chi_square cs(docs);
    auto cs_features = top_features(cs.select(), .1);
    reduced = Document::filter_features(docs, cs_features);
    cv(svm, reduced);

    // sLDA
    cout << "Running sLDA..." << endl;
    topics::slda lda(config["slda"], .1);
    lda.estimate(docs);
    auto lda_features = top_features(lda.select_features(), .1);
    reduced = Document::filter_features(docs, lda_features);
    cv(svm, reduced);

    return 0;
}
