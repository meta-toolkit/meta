/**
 * @file file_corpus.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FILE_CORPUS_H_
#define META_FILE_CORPUS_H_

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
 * Creates document objects from individual files, each representing a single
 * document.
 */
class file_corpus : public corpus
{
  public:
    /// The identifier for this corpus
    const static util::string_view id;

    /**
     * @param prefix Path to where the files are located
     * @param doc_list A file containing the path to each document in the
     * corpus preceded by a class label (or "[none]")
     * @param encoding The encoding of the corpus
     */
    file_corpus(const std::string& prefix, const std::string& doc_list,
                std::string encoding);

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

    /**
     * @return the metadata schema for this corpus
     */
    metadata::schema_type schema() const override;

  private:
    /// the current document we are on
    uint64_t cur_;

    /// the path to all the documents
    std::string prefix_;

    /// contains doc class labels and paths
    std::vector<std::pair<std::string, class_label>> docs_;
};

/**
 * Specialization of the factory method used to create line_corpus
 * instances.
 */
template <>
std::unique_ptr<corpus> make_corpus<file_corpus>(util::string_view prefix,
                                                 util::string_view dataset,
                                                 const cpptoml::table& config);
}
}
#endif
