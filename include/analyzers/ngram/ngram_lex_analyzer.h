/**
 * @file ngram_lex_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_LEX_TOKENIZER_H_
#define _NGRAM_LEX_TOKENIZER_H_

#include "analyzers/analyzer_factory.h"
#include "analyzers/ngram/ngram_simple_analyzer.h"
#include "util/clonable.h"

namespace meta {
namespace analyzers {

class ngram_lex_analyzer
    : public util::multilevel_clonable<analyzer, ngram_simple_analyzer,
                                       ngram_lex_analyzer>
{
    using base = util::multilevel_clonable<analyzer, ngram_simple_analyzer,
                                           ngram_lex_analyzer>;
    public:
        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_lex_analyzer(uint16_t n);

        /**
         * Tokenizes a file into a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) override;

        /**
         * Identifier for this analyzer.
         */
        const static std::string id;
};

/**
 * Specialization of the factory method for creating ngram_lex_analyzers.
 */
template <>
std::unique_ptr<analyzer>
    make_analyzer<ngram_lex_analyzer>(const cpptoml::toml_group&,
                                      const cpptoml::toml_group&);
}
}

#endif
