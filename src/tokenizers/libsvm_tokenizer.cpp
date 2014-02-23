/**
 * @file libsvm_tokenizer.cpp
 * @author Sean Massung
 */

#include <sstream>

#include "corpus/document.h"
#include "io/libsvm_parser.h"
#include "tokenizers/libsvm_tokenizer.h"

namespace meta {
namespace tokenizers {

void libsvm_tokenizer::tokenize(corpus::document & doc)
{
    for(auto & count_pair: io::libsvm_parser::counts(doc.content(), false))
        doc.increment(std::to_string(count_pair.first), count_pair.second);

    // label info is inside the document content for libsvm format; the line
    // corpus will not set it since it's not in a separate file
    doc.set_label(io::libsvm_parser::label(doc.content()));
}

}
}
