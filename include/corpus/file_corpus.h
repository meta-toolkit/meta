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
#include <vector>
#include <utility>
#include "corpus/corpus.h"

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

  private:
    /// the current document we are on
    uint64_t cur_;

    /// the path to all the documents
    std::string prefix_;

    /// contains doc class labels and paths
    std::vector<std::pair<std::string, class_label>> docs_;
};
}
}

#endif
