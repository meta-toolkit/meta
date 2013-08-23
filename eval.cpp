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
#include "classify/classifier/liblinear_svm.h"
#include "io/config_reader.h"

using std::cerr;
using std::endl;
using namespace meta;

void run(const cpptoml::toml_group & config)
{
    std::string prefix = *cpptoml::get_as<std::string>( config, "prefix" ) 
        + *cpptoml::get_as<std::string>( config, "dataset" );
    std::string corpus_file = prefix 
        + "/" 
        + *cpptoml::get_as<std::string>( config, "list" )
        + "-test.txt";
    std::shared_ptr<tokenizers::tokenizer> tok = io::config_reader::create_tokenizer(config);
    std::vector<index::document> test_docs = index::document::load_docs(corpus_file, prefix);

    size_t i = 0;
    for(auto & d: test_docs)
    {
        common::show_progress(i++, test_docs.size(), 20, "  tokenizing ");
        tok->tokenize(d);
    }
    common::end_progress("  tokenizing ");

    classify::liblinear_svm svm{ *cpptoml::get_as<std::string>( config, "liblinear" ) };
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
    auto config = io::config_reader::read(filename);
    
    auto tokenizers = config.get_group_array( "tokenizers" );
    tokenizers->array().clear();
    tokenizers->array().push_back( std::make_shared<cpptoml::toml_group>() );
    tokenizers->array().push_back( std::make_shared<cpptoml::toml_group>() );

    tokenizers->array()[0]->insert<std::string>( "method", "ngram" );
    tokenizers->array()[1]->insert<std::string>( "method", "tree" );

    for(auto & treeOpt: {"Subtree", "Skel", "Semi", "Tag"})
    {
        tokenizers->array()[1]->insert<std::string>( "treeOpt", treeOpt );
        for(auto & ngramOpt: {"Word" })
        {
            for(size_t i = 1; i < 5; ++i)
            {
                tokenizers->array()[0]->insert<std::string>( "ngramOpt", ngramOpt );
                tokenizers->array()[0]->insert<int64_t>( "ngram", i );
                run(config);
            }
        }
    }
}

void run_best_kaggle(const std::string & filename)
{
    // init config file
    auto config = io::config_reader::read(filename);
    
    auto tokenizers = config.get_group_array( "tokenizers" );
    tokenizers->array().clear();
    tokenizers->array().push_back( std::make_shared<cpptoml::toml_group>() );
    tokenizers->array().push_back( std::make_shared<cpptoml::toml_group>() );

    auto & ngram = tokenizers->array()[0];
    auto & tree = tokenizers->array()[1];

    // run best n-gram as determined by training
    ngram->insert<std::string>( "method", "ngram" );
    tree->insert<std::string>( "method", "tree" );

    //for(auto & opt: {"Subtree", "Skel", "Semi", "Tag"})
    for(auto & opt: {"Depth", "Branch"})
    {
        tree->insert<std::string>( "treeOpt", opt );
        /// @todo the comments don't match the numbers here...
        // unigram words
        ngram->insert<std::string>( "ngramOpt", "Word" );
        ngram->insert<int64_t>( "ngram", 2 );
        run(config);

        // bigram POS tags
        ngram->insert<std::string>( "ngramOpt", "POS" );
        ngram->insert<int64_t>( "ngram", 2 );
        run(config);

        // bigram function words
        ngram->insert<std::string>( "ngramOpt", "FW" );
        ngram->insert<int64_t>( "ngram", 1 );
        run(config);
    }
}

void run_best_sentiment(const std::string & filename)
{
    // init config file
    auto config = io::config_reader::read(filename);
    
    auto tokenizers = config.get_group_array( "tokenizers" );
    tokenizers->array().clear();
    tokenizers->array().push_back( std::make_shared<cpptoml::toml_group>() );
    tokenizers->array().push_back( std::make_shared<cpptoml::toml_group>() );

    auto & ngram = tokenizers->array()[0];
    auto & tree = tokenizers->array()[1];

    // run best n-gram as determined by training
    ngram->insert<std::string>( "method", "ngram" );
    tree->insert<std::string>( "method", "tree" );

    for(auto & opt: {"Subtree", "Skel", "Semi", "Tag"})
    {
        tree->insert<std::string>( "treeOpt", opt );
        
        /// @todo the numbers don't match the comments here...
        // unigram words
        ngram->insert<std::string>( "ngramOpt", "Word" );
        ngram->insert<int64_t>( "ngram", 1 );
        run(config);

        // bigram POS tags
        ngram->insert<std::string>( "ngramOpt", "POS" );
        ngram->insert<int64_t>( "ngram", 3 );
        run(config);

        // bigram function words
        ngram->insert<std::string>( "ngramOpt", "FW" );
        ngram->insert<int64_t>( "ngram", 1 );
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

    auto config = io::config_reader::read(argv[1]);
    run(config);
    
    //run_best_ceeaus(argv[1]);
    //run_best_kaggle(argv[1]);
    //run_best_sentiment(argv[1]);

    return 0;
}
