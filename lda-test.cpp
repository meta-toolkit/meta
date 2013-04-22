#include <iostream>
#include <string>
#include <vector>

#include "index/document.h"
#include "tokenizers/ngram/ngram_tokenizer.h"
#include "topics/lda_gibbs.h"
#include "topics/parallel_lda_gibbs.h"
#include "topics/lda_cvb.h"

using namespace meta;

int print_usage( const std::string & name ) {
    std::cout << "Usage: " << name << " type prefix/full-corpus.txt prefix alpha beta topics\n"
        "\tRuns LDA of the given type (gibbs, pargibbs, cvb) on the given"
        " corpus, with hyperparameters alpha" " and beta, and topics number"
        " of topics" << std::endl;
    return 1;
}

template <class Model>
int run_lda( std::vector<index::document> & docs, size_t topics, double alpha, double beta ) {
    Model model{ docs, topics, alpha, beta };
    model.run( 1000 );
    model.save( "lda_model" );
    return 0;
}

int run_lda( const std::string & type, const std::string & filename,
             const std::string & prefix, double alpha, double beta, 
             size_t topics ) {
    using namespace meta::topics;
    std::cout << "Loading documents...\r" << std::flush;
    std::vector<index::document> docs = index::document::load_docs( filename, prefix );
    if( type == "gibbs" ) {
        std::cout<< "Beginning LDA using serial Gibbs sampling..." << std::endl;
        return run_lda<lda_gibbs>( docs, topics, alpha, beta );
    } else if( type == "pargibbs" ) {
        std::cout<< "Beginning LDA using parallel Gibbs sampling..." << std::endl;
        return run_lda<parallel_lda_gibbs>( docs, topics, alpha, beta );
    } else if( type == "cvb" ) {
        std::cout<< "Beginning LDA using serial collapsed variational bayes..." << std::endl;
        return run_lda<lda_cvb>( docs, topics, alpha, beta );
    }
    std::cout << "Incorrect method selected: must be gibbs, pargibbs, or cvb" << std::endl;
    return 1;
}

int main( int argc, char ** argv ) {
    if( argc < 7 )
        return print_usage( argv[0] );
    std::vector<std::string> args( argv, argv + argc );
    double alpha = std::stod( argv[4] );
    double beta = std::stod( argv[5] );
    size_t topics = std::stoul( argv[6] );
    std::cout << "alpha: " << alpha << "\nbeta: " << beta << "\ntopics: " << topics << std::endl;
    return run_lda( args[1], args[2], args[3], alpha, beta, topics );
}
