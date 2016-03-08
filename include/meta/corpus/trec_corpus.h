/**
 * @file trec_corpus.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TREC_CORPUS_H_
#define META_TREC_CORPUS_H_

#include "meta/corpus/corpus.h"
#include "meta/corpus/corpus_factory.h"

namespace meta
{
namespace corpus
{
/**
 * Parses files in the TREC file format.
 * @see http://trec.nist.gov/
 * @see https://sourceforge.net/p/lemur/wiki/Indexer%20File%20Formats/
 *
 * TREC datasets are distributed as collections of .gz files. Each .gz file has
 * a number of <DOC>...</DOC> tags containing text or Web documents to index.
 * The actual name of the tags may vary, so these are specified as corpus config
 * options.
 *
 * The total number of documents to index must be known. This is set as the
 * `num-docs` required parameter in the corpus config. This number may be set to
 * less than the total number of actual documents in the TREC collection, but
 * not more.
 *
 * Finally, additional metadata tags such as <DOCHDR> can be skipped
 * during indexing; these are also specified as arrays of strings in the corpus
 * config file.
 *
 * Required config parameters:
 * ~~~toml
 * type = "trec-corpus"
 * num-docs = 1000
 * file-list = "path/to/all/.gz/files"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * doc-tag = "DOC"         # default
 * name-tag = "DOCNO"      # default
 * skip-tags = []          # default
 * store-full-text = false # default
 * ~~~
 */
class trec_corpus : public corpus
{
  public:
    /// The identifier for this corpus
    const static util::string_view id;

    /**
     * @param prefix The path to the corpus files
     * @param file_list_path The path to the file that contains paths to each
     * .gz TREC file
     * @param encoding The encoding for the TREC documents
     * @param num_docs The total number of documents in this corpus
     * @param doc_tag Name of the tag that separates TREC docs
     * @param name_tag Name of the tag that holds the doc name (TREC id)
     * @param skip_tags A list of tags to ignore in the TREC document
     */
    trec_corpus(const std::string& prefix, const std::string& file_list_path,
                std::string encoding, uint64_t num_docs,
                const std::string& doc_tag = "DOC",
                const std::string& name_tag = "DOCNO",
                const std::vector<std::string>& skip_tags = {});

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
    metadata::schema schema() const override;

  private:
    /**
     * Move to the next <DOC> in the current file, opening the next file if
     * necessary.
     */
    void advance();

    /// Path prefix to this corpus
    std::string prefix_;

    /// The current document we are on
    doc_id cur_id_;

    /// The number of lines in the file
    uint64_t num_docs_;

    /// The current file being parsed
    uint64_t file_idx_;

    /// Holds the current document being parsed
    std::string buffer_;

    /// Current position in the string buffer
    uint64_t buf_idx_;

    /// List of .gz filenames containing multiple TREC documents
    std::vector<std::string> filenames_;

    /// Name of the tag that separates documents
    std::string doc_start_tag_;
    std::string doc_end_tag_;

    /// Name of the tag that holds the document name (TREC id)
    std::string name_start_tag_;
    std::string name_end_tag_;

    /// Tags in the TREC doc to ignore
    std::vector<std::string> skip_tags_;
};

/**
 * Specialization of the factory method used to create trec_corpus instances.
 */
template <>
std::unique_ptr<corpus> make_corpus<trec_corpus>(util::string_view prefix,
                                                 util::string_view dataset,
                                                 const cpptoml::table& config);
}
}
#endif
