/**
 * @file libsvm_tokenizer.cpp
 * @author Sean Massung
 */

#include <sstream>
#include "tokenizers/libsvm_tokenizer.h"

namespace meta {
namespace tokenizers {

void libsvm_tokenizer::tokenize(corpus::document & doc)
{
    std::stringstream stream{doc.content()};
    std::string token;
    
    // first token is the class label
    stream >> token;
    doc.set_label(class_label{token});

    // remaining tokens are "feature:feature_count"
    while(stream >> token)
    {
        size_t idx = token.find_first_of(':');
        std::string feature = token.substr(0, idx);
        double count;
        std::istringstream{token.substr(idx + 1)} >> count;
        doc.increment(feature, count);
    }
}

}
}
