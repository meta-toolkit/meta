/**
 * @file lda_model.cpp
 */

#include "topics/lda_model.h"
#include "util/common.h"

namespace meta {
namespace topics {

using corpus::document;

lda_model::lda_model( std::vector<document> & docs, size_t num_topics,
        const std::shared_ptr<tokenizers::tokenizer> & tokenizer ):
    docs_( docs ),
    tokenizer_{ tokenizer },
    num_topics_{ num_topics },
    num_words_{tokenizer_->num_terms()}
{ /* nothing */ }

void lda_model::save_doc_topic_distributions( const std::string & filename ) const {
    std::ofstream file{ filename };
    for( doc_id i{ 0 }; i < docs_.size(); ++i ) {
        file << docs_[i].name() << "\t";
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
        for( const auto & pair : tokenizer_->term_id_mapping() ) {
            term_id term = pair.first;
            double prob = compute_term_topic_probability( term, j );
            if( prob > 0 )
                file << term << ":" << prob << "\t";
        }
        file << "\n";
    }
}

std::vector<std::pair<term_id, double>> lda_model::select() const
{
    std::unordered_map<term_id, double> terms;
    for(size_t j = 0; j < num_topics_; ++j)
    {
        for(auto & pair: tokenizer_->term_id_mapping())
        {
            term_id term = pair.first;
            double prob = compute_term_topic_probability( term, j );
            if(terms[term] == 0 || terms[term] < prob)
                terms[term] = prob;
        }
    }

    std::vector<std::pair<term_id, double>> ret{terms.begin(), terms.end()};
    std::sort(ret.begin(), ret.end(),
        [](const std::pair<term_id, double> & a, const std::pair<term_id, double> & b) {
            return a.second > b.second;
        }
    );

    return ret; 
}

void lda_model::save_term_mapping( const std::string & filename ) const {
    tokenizer_->save_term_id_mapping( filename );
}

void lda_model::save( const std::string & prefix ) const {
    save_doc_topic_distributions( prefix + ".theta" );
    save_topic_term_distributions( prefix + ".phi" );
    save_term_mapping( prefix + ".terms" );
}

}
}
