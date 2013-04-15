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
#include "tokenizers/ngram_word_tokenizer.h"
#include "classify/liblinear_svm.h"
#include "io/config_reader.h"

using std::cerr;
using std::endl;
using namespace meta;

void run(const std::unordered_map<std::string, std::string> & config)
{
    std::string prefix = config.at("prefix") + config.at("dataset");
    std::shared_ptr<tokenizers::tokenizer> tok = io::config_reader::create_tokenizer(config);
    std::vector<index::Document> dev_docs = index::Document::loadDocs(prefix + "/train.txt", prefix);
    size_t i = 0;
    for(auto & d: dev_docs)
    {
        common::show_progress(i++, dev_docs.size(), 20, "  tokenizing ");
        tok->tokenize(d, nullptr);
    }
    common::end_progress("  tokenizing ");

    classify::liblinear_svm svm(config.at("liblinear"));
    classify::confusion_matrix matrix = svm.cross_validate(dev_docs, 5);

    std::string filename = io::config_reader::get_config_string(config) + ".txt";
    std::cerr << std::endl << "Saving results to " << filename << std::endl;

    std::ofstream outfile(filename);
    std::stringstream ss;
    //matrix.print_stats(ss); // ceeaus, sentiment
    matrix.print_result_pairs(ss); // kaggle (for quadratic weighted kappa input)
    cerr << ss.str();
    outfile << ss.str();
    outfile.close();
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.ini" << endl;
        return 1;
    }

    std::unordered_map<std::string, std::string> config = io::config_reader::read(argv[1]);

 // for(auto & opt: {"Branch", "Subtree", "Depth", "Skel", "Semi", "Tag"})
 // {
 //     config["treeOpt_1"] = opt;
 //     run(config);
 // }

    for(auto & ngramOpt: {"Word", "Char", "FW", "POS"})
    {
        config["ngramOpt_1"] = ngramOpt;
        for(size_t n = 1; n < 6; ++n)
        {
            config["ngram_1"] = common::to_string(n);
            run(config);
        }
    }
 
    return 0;
}
