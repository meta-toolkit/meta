/**
 * @file analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <stdexcept>
#include <memory>

#include "io/parser.h"

namespace cpptoml {
class toml_group;
}

namespace meta {

namespace corpus {
class document;
}

namespace analyzers {

/**
 * An class that provides a framework to produce token counts from documents.
 * All analyzers inherit from this class and (possibly) implement tokenize().
 */
class analyzer
{
    public:
        /**
         * A default virtual destructor.
         */
        virtual ~analyzer() = default;

        /**
         * Tokenizes a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) = 0;

        /**
         * @return a Tokenizer as specified by a config object
         */
        static std::unique_ptr<analyzer> load(
                const cpptoml::toml_group & config);

        /**
         * @param doc The document to parse
         * @param extension The possible file extension for this document if it
         * is represented by a file on disk
         * @param delims Possible character delimiters to use when parsing the
         * file
         * @return a parser suited to read data that this document represents
         */
        static io::parser create_parser(const corpus::document & doc,
                const std::string & extension, const std::string & delims);

    public:
        /**
         * Basic exception for analyzer interactions.
         */
        class analyzer_exception: public std::runtime_error
        {
            public:
                using std::runtime_error::runtime_error;
        };
};

}
}

#endif
