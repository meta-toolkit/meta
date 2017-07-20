/**
 * @file gz_corpus.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GZ_CORPUS_H_
#define META_GZ_CORPUS_H_

#include "meta/config.h"
#include "meta/corpus/corpus.h"
#include "meta/corpus/corpus_factory.h"
#include "meta/io/gzstream.h"

namespace meta
{
namespace corpus
{

/**
 * Fills document objects with content line-by-line from gzip-compressed
 * input files.
 */
class gz_corpus : public corpus
{
  public:
    /// The identifier for this corpus
    const static util::string_view id;

    /**
     * @param file The path to the compressed corpus file, where each line
     * represents a document
     * @param encoding The encoding for the file
     * @param num_docs The number of documents in this corpus
     */
    gz_corpus(const std::string& file, std::string encoding, uint64_t num_docs);

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

    /// The stream for reading the corpus
    io::gzifstream corpus_stream_;

    /// The stream to read the class labels
    io::gzifstream class_stream_;
};

/**
 * Specialization of the factory method used to create gz_corpus instances.
 */
template <>
std::unique_ptr<corpus> make_corpus<gz_corpus>(util::string_view prefix,
                                               util::string_view dataset,
                                               const cpptoml::table& config);
}
}
#endif
