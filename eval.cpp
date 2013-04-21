/**
 * @file eval.cpp
 * @author Sean Massung
 *
 * Finds the best n-value for a given feature by performing cross-validation on
 * a development set.
 */

#include <fstream>
#include <sstream>
#include "index/document.h"
#include "classify/liblinear_svm.h"
#include "io/config_reader.h"

using std::cerr;
using std::endl;
using namespace meta;

void run(const std::unordered_map<std::string, std::string> & config)
{
    std::string prefix = config.at("prefix") + config.at("dataset");
    std::shared_ptr<tokenizers::tokenizer> tok = io::config_reader::create_tokenizer(config);
    std::vector<index::Document> test_docs = index::Document::loadDocs(prefix + "/test.txt", prefix);

    size_t i = 0;
    for(auto & d: test_docs)
    {
        common::show_progress(i++, test_docs.size(), 20, "  tokenizing ");
        tok->tokenize(d, nullptr);
    }
    common::end_progress("  tokenizing ");

    classify::liblinear_svm svm(config.at("liblinear"));
    classify::confusion_matrix matrix = svm.cross_validate(test_docs, 5);

    std::string filename = io::config_reader::get_config_string(config) + ".txt";
    std::cerr << "Saving results to " << filename << std::endl << std::endl;

    std::ofstream outfile(filename);
    std::stringstream ss;
    matrix.print_stats(ss); // ceeaus, sentiment
    //matrix.print_result_pairs(ss); // kaggle (for quadratic weighted kappa input)
    cerr << ss.str();
    outfile << ss.str();
    outfile.close();
}

void run_best_ceeaus(const std::string & filename)
{
    std::unordered_map<std::string, std::string> config = io::config_reader::read(filename);

    config["method_1"] = "ngram";
    config["method_2"] = "tree";

    for(auto & treeOpt: {"Subtree", "Skel", "Semi", "Tag"})
    {
        config["treeOpt_2"] = treeOpt;

        for(auto & ngramOpt: {"Word" })
        {
            for(size_t i = 1; i < 5; ++i)
            {
                config["ngramOpt_1"] = ngramOpt;
                config["ngram_1"] = common::to_string(i);
                run(config);
            }
        }
    }
}

void run_best_kaggle(const std::string & filename)
{
    // init config file
    std::unordered_map<std::string, std::string> config = io::config_reader::read(filename);

    // run best n-gram as determined by training
    config["method_1"] = "ngram";
    config["method_2"] = "tree";

    //for(auto & opt: {"Subtree", "Skel", "Semi", "Tag"})
    for(auto & opt: {"Depth", "Branch"})
    {
        config["treeOpt_2"] = opt;

        // unigram words
        config["ngramOpt_1"] = "Word";
        config["ngram_1"] = "2";
        run(config);

        // bigram POS tags
        config["ngramOpt_1"] = "POS";
        config["ngram_1"] = "2";
        run(config);

        // bigram function words
        config["ngramOpt_1"] = "FW";
        config["ngram_1"] = "1";
        run(config);
    }
}

void run_best_sentiment(const std::string & filename)
{
    // init config file
    std::unordered_map<std::string, std::string> config = io::config_reader::read(filename);

    // run best n-gram as determined by training
    config["method_1"] = "ngram";
    config["method_2"] = "tree";

    for(auto & opt: {"Subtree", "Skel", "Semi", "Tag"})
    {
        config["treeOpt_2"] = opt;

        // unigram words
        config["ngramOpt_1"] = "Word";
        config["ngram_1"] = "1";
        run(config);

        // bigram POS tags
        config["ngramOpt_1"] = "POS";
        config["ngram_1"] = "3";
        run(config);

        // bigram function words
        config["ngramOpt_1"] = "FW";
        config["ngram_1"] = "1";
        run(config);
    }
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.ini" << endl;
        return 1;
    }

    std::unordered_map<std::string, std::string> config = io::config_reader::read(argv[1]);
    run(config);
    
    //run_best_ceeaus(argv[1]);
    //run_best_kaggle(argv[1]);
    //run_best_sentiment(argv[1]);

    return 0;
}
