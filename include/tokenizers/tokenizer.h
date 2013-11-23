/**
 * @file tokenizer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <memory>
#include <string>
#include "meta.h"
#include "cpptoml.h"
#include "corpus/document.h"

namespace meta {
namespace tokenizers {

/**
 * An class that provides a framework to produce token counts from documents.
 * All tokenizers inherit from this class and (possibly) implement tokenize().
 */
class tokenizer
{
    public:
        /**
         * A default virtual destructor.
         */
        virtual ~tokenizer() = default;

        /**
         * Tokenizes a document.
         * @param doc The document to store the tokenized information in
         */
        virtual void tokenize(corpus::document & doc) = 0;

        /**
         * @return a Tokenizer as specified by a config object
         */
        static std::unique_ptr<tokenizer> load(
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
         * Basic exception for tokenizer interactions.
         */
        class tokenizer_exception: public std::exception
        {
            public:
                tokenizer_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }

            private:
                std::string _error;
        };
};

}
}

#endif
