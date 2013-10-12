/**
 * @file line_corpus.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _LINE_CORPUS_H_
#define _LINE_CORPUS_H_

#include <string>
#include <vector>
#include <utility>
#include "io/parser.h"
#include "corpus/corpus.h"

namespace meta {
namespace corpus {

/**
 * Fills document objects with content line-by-line from an input file. It is up
 * to the tokenizer used to be able to correctly parse the document content into
 * labels and features.
 */
class line_corpus: public corpus
{
    public:
        /**
         * @param file The path to the corpus file, where each line represents
         * a document
         */
        line_corpus(const std::string & file);

        /**
         * @return whether there is another document in this corpus
         */
        bool has_next() const override;

        /**
         * @return the next document from this corpus
         */
        document next() override;

        /**
         * @return the number of documents in this corpus
         */
        uint64_t size() const override;

    private:
        /** the current document we are on */
        uint64_t _cur;

        /** the number of lines in the file */
        uint64_t _num_lines;

        /** parser to read the corpus file */
        io::parser _parser;
};

}
}

#endif
