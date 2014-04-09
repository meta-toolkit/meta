/**
 * @file line_corpus.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LINE_CORPUS_H_
#define META_LINE_CORPUS_H_

#include <string>
#include <vector>
#include <utility>
#include "io/parser.h"
#include "corpus/corpus.h"

namespace meta
{
namespace corpus
{

/**
 * Fills document objects with content line-by-line from an input file. It is up
 * to the tokenizer used to be able to correctly parse the document content into
 * labels and features.
 */
class line_corpus : public corpus
{
  public:
    /**
     * @param file The path to the corpus file, where each line represents
     * a document
     * @param encoding The encoding for the file
     * @param num_lines The number of lines in the corpus file if known
     * beforehand. If unknown, leave out this parameter and the value will
     * be calculated in the constructor.
     */
    line_corpus(const std::string& file, std::string encoding,
                uint64_t num_lines = 0);

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
    /// The current document we are on
    doc_id cur_id_;

    /// The number of lines in the file
    uint64_t num_lines_;

    /// Parser to read the corpus file
    io::parser parser_;

    /// Parser to read the class labels
    std::unique_ptr<io::parser> class_parser_;

    /// Parser to read the document names
    std::unique_ptr<io::parser> name_parser_;
};
}
}

#endif
