/**
 * @file libsvm_corpus.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CORPUS_LIBSVM_CORPUS_H_
#define META_CORPUS_LIBSVM_CORPUS_H_

#include <fstream>

#include "meta/config.h"
#include "meta/corpus/corpus.h"
#include "meta/corpus/corpus_factory.h"

namespace meta
{
namespace corpus
{

/**
 * Fills document objects with content line-by-line from a
 * libsvm-formatted input file. This should **only** be used with a
 * libsvm_analyzer.
 */
class libsvm_corpus : public corpus
{
  public:
    /// The identifier for this corpus
    const static util::string_view id;

    /// The label type for the corpus
    enum class label_type
    {
        CLASSIFICATION,
        REGRESSION
    };

    /**
     * @param file The path to the corpus file
     * @param type The label type for the data (classification or
     * regression)
     * @param num_docs The number of documents (i.e., lines) in the corpus file
     * if known beforehand. If unknown, leave out this parameter and the value
     * will be calculated in the constructor.
     */
    libsvm_corpus(const std::string& file,
                  label_type type = label_type::CLASSIFICATION,
                  uint64_t num_docs = 0);

    bool has_next() const override;

    document next() override;

    uint64_t size() const override;

    metadata::schema_type schema() const override;

  private:
    /// The current document we are on
    doc_id cur_id_;

    /// The label type
    label_type lbl_type_;

    /// The number of lines in the file
    uint64_t num_lines_;

    /// The next document
    std::string next_content_;

    /// The stream being read from
    std::ifstream input_;
};

/**
 * Specialization of the factory method used to create libsvm_corpus
 * instances.
 */
template <>
std::unique_ptr<corpus>
make_corpus<libsvm_corpus>(util::string_view prefix, util::string_view dataset,
                           const cpptoml::table& config);
}
}
#endif
