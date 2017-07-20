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

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "meta/config.h"
#include "meta/corpus/corpus.h"
#include "meta/corpus/corpus_factory.h"

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
    /// The identifier for this corpus
    const static util::string_view id;

    /**
     * @param file The path to the corpus file, where each line represents
     * a document
     * @param encoding The encoding for the file
     * @param num_docs The number of documents (i.e., lines) in the corpus file
     * if known beforehand. If unknown, leave out this parameter and the value
     * will be calculated in the constructor.
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
    std::ifstream infile_;

    /// Parser to read the class labels
    std::unique_ptr<std::ifstream> class_infile_;
};

/**
 * Specialization of the factory method used to create line_corpus
 * instances.
 */
template <>
std::unique_ptr<corpus> make_corpus<line_corpus>(util::string_view prefix,
                                                 util::string_view dataset,
                                                 const cpptoml::table& config);
}
}
#endif
