#include <iostream>
#include <string>
#include <vector>

#include "index/document.h"
#include "tokenizers/ngram_tokenizer.h"
#include "topics/lda_gibbs.h"

int print_usage( const std::string & name ) {
    std::cout << "Usage: " << name << " prefix/full-corpus.txt prefix alpha beta topics\n"
        "\tRuns LDA with Gibbs Sampling on the given corpus, with hyperparameters alpha"
        " and beta, and topics number of topics" << std::endl;
    return 1;
}

int run_lda( const std::string & filename, const std::string & prefix,
             double alpha, double beta, size_t topics ) {
    using namespace topics;
    std::cout << "Loading documents...\r" << std::flush;
    std::vector<Document> docs = Document::loadDocs( filename, prefix );
    std::cout<< "Beginning LDA using Gibbs sampling..." << std::endl;
    lda_gibbs model{ docs, topics, alpha, beta };
    model.run( 1000 );
    model.save( "lda_model" );
    
    return 0;
}

int main( int argc, char ** argv ) {
    if( argc < 6 )
        return print_usage( argv[0] );
    std::vector<std::string> args( argv, argv + argc );
    double alpha = std::stod( argv[3] );
    double beta = std::stod( argv[4] );
    size_t topics = std::stoul( argv[5] );
    std::cout << "alpha: " << alpha << "\nbeta: " << beta << "\ntopics: " << topics << std::endl;
    return run_lda( args[1], args[2], alpha, beta, topics );
}
