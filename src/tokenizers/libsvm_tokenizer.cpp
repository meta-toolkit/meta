/**
 * @file libsvm_tokenizer.cpp
 * @author Sean Massung
 */

#include <sstream>

#include "corpus/document.h"
#include "io/libsvm_parser.h"
#include "tokenizers/libsvm_tokenizer.h"
#include "util/common.h"

namespace meta {
namespace tokenizers {

void libsvm_tokenizer::tokenize(corpus::document & doc)
{
    for(auto & count_pair: io::libsvm_parser::counts(doc.content(), false))
        doc.increment(common::to_string(count_pair.first), count_pair.second);
}

}
}
