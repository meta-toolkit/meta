/**
 * @file libsvm_analyzer.cpp
 * @author Sean Massung
 */

#include <sstream>

#include "corpus/document.h"
#include "io/libsvm_parser.h"
#include "analyzers/libsvm_analyzer.h"

namespace meta
{
namespace analyzers
{

const std::string libsvm_analyzer::id = "libsvm";

void libsvm_analyzer::tokenize(corpus::document& doc)
{
    for (auto& count_pair : io::libsvm_parser::counts(doc.content(), false))
        doc.increment(std::to_string(count_pair.first), count_pair.second);

    // label info is inside the document content for libsvm format; the line
    // corpus will not set it since it's not in a separate file
    doc.label(io::libsvm_parser::label(doc.content()));
}
}
}
