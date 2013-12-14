/**
 * @file libsvm_tokenizer.cpp
 * @author Sean Massung
 */

#include <sstream>
#include "tokenizers/libsvm_tokenizer.h"

namespace meta {
namespace tokenizers {

void libsvm_tokenizer::tokenize_document(corpus::document & document,
        std::function<term_id(const std::string &)> mapping)
{
    std::stringstream stream{document.content()};
    std::string token;
    
    // remaining tokens are "feature:feature_count"
    while(stream >> token)
    {
        size_t idx = token.find_first_of(':');
        std::string feature = token.substr(0, idx);
        double count;
        std::istringstream{token.substr(idx + 1)} >> count;
        document.increment(mapping(feature), count);
    }
}

}
}
