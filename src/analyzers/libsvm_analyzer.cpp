/**
 * @file libsvm_analyzer.cpp
 * @author Sean Massung
 */

#include <sstream>

#include "corpus/document.h"
#include "io/libsvm_parser.h"
#include "analyzers/libsvm_analyzer.h"

namespace meta {
namespace analyzers {

void libsvm_analyzer::tokenize(corpus::document & doc)
{
    for(auto & count_pair: io::libsvm_parser::counts(doc.content(), false))
        doc.increment(std::to_string(count_pair.first), count_pair.second);
}

}
}
