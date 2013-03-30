#include <iostream>
#include <string>
#include <vector>

#include "cluster/agglomerative_clustering.h"
#include "cluster/basic_single_link_policy.h"
#include "cluster/similarity.h"
#include "index/document.h"
#include "tokenizers/ngram_tokenizer.h"

void run_test( const std::string & filename, const std::string & prefix ) {
    using namespace meta;
    using namespace meta::clustering;
    tokenizers::NgramTokenizer t(1, tokenizers::NgramTokenizer::Word);
    
    std::cout << "Loading documents...\r" << std::flush;
    std::vector<index::Document> docs = index::Document::loadDocs( filename, prefix );
    std::cout << "Tokenizing documents...\r" << std::flush;
    for( auto & d : docs )
        t.tokenize( d );

    std::cout << "Clustring documents..." << std::endl;
    agglomerative_clustering<index::Document,
                             basic_single_link_policy<similarity::cosine>>
    cluster( docs );
}

int main( int argc, char ** argv ) {
    std::vector<std::string> args( argv, argv + argc );
    run_test( args[1], args[2] );
    return 0;
}
