#include <vector>
#include <string>

#include "cluster/point.h"
#include "tokenizers/ngram_tokenizer.h"

void run_test( const std::string & filename, const std::string & prefix ) {
    NgramTokenizer t(1, NgramTokenizer::Word);
    
    std::vector<Document> docs = Document::loadDocs( filename, prefix );
    for( auto & d : docs )
        t.tokenize( d );

    clustering::point<TermID, Document> avg{ docs[0] };
    for( auto & d : docs ) {
        clustering::point<TermID, Document> p{ d };
        avg = clustering::merge_points( avg, p );
    }
}

int main( int argc, char ** argv ) {
    std::vector<std::string> args( argv, argv + argc );
    run_test( args[1], args[2] );
    return 0;
}
