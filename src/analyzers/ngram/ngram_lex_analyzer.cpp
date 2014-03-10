/**
 * @file ngram_lex_analyzer.cpp
 */

#include "cpptoml.h"
#include "analyzers/ngram/ngram_lex_analyzer.h"

namespace meta
{
namespace analyzers
{

const std::string ngram_lex_analyzer::id = "ngram-lex";

ngram_lex_analyzer::ngram_lex_analyzer(uint16_t n) : base{n}
{/* nothing */
}

void ngram_lex_analyzer::tokenize(corpus::document& doc)
{
    io::parser parser{create_parser(doc, ".lex", " \n")};
    simple_tokenize(parser, doc);
}

template <>
std::unique_ptr<analyzer> make_analyzer
    <ngram_lex_analyzer>(const cpptoml::toml_group&,
                         const cpptoml::toml_group& config)
{
    if (auto n_val = config.get_as<int64_t>("ngram"))
        return make_unique<ngram_lex_analyzer>(*n_val);
    throw analyzer::analyzer_exception{
        "ngram size needed for ngram lex analyzer in config file"};
}
}
}
