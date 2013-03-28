/**
 * @file lda_model.cpp
 */

#include "topics/lda_model.h"
#include "util/common.h"

namespace topics {

lda_model::lda_model( std::vector<Document> & docs, size_t num_topics )
        : docs_{ docs }, tokenizer_{ 1, NgramTokenizer::Word },
          num_topics_{ num_topics } {
    for( size_t i = 0; i < docs_.size(); ++i ) {
        Common::show_progress( i, docs_.size(), 10, "Tokenizing documents: " );
        tokenizer_.tokenize( docs_[i] );
    }
    std::cerr << '\n';
    num_words_ = tokenizer_.getNumTerms();
}

void lda_model::save_doc_topic_distributions( const std::string & filename ) const {
    std::ofstream file{ filename };
    for( size_t i = 0; i < docs_.size(); ++i ) {
        file << docs_[i].getName() << "\t";
        for( size_t j = 0; j < num_topics_; ++j ) {
            double prob = compute_doc_topic_probability( i, j );
            if( prob > 0 )
                file << j << ":" << prob << "\t";
        }
        file << "\n";
    }
}

void lda_model::save_topic_term_distributions( const std::string & filename ) const {
    std::ofstream file{ filename };
    for( size_t j = 0; j < num_topics_; ++j ) {
        file << j << "\t";
        for( const auto & pair : tokenizer_.getTermIDMapping() ) {
            TermID term = pair.first;
            double prob = compute_term_topic_probability( term, j );
            if( prob > 0 )
                file << term << ":" << prob << "\t";
        }
        file << "\n";
    }
}

void lda_model::save_term_mapping( const std::string & filename ) const {
    tokenizer_.saveTermIDMapping( filename );
}

void lda_model::save( const std::string & prefix ) const {
    save_doc_topic_distributions( prefix + ".theta" );
    save_topic_term_distributions( prefix + ".phi" );
    save_term_mapping( prefix + ".terms" );
}

}
