/**
 * @file corpus.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CORPUS_H_
#define META_CORPUS_H_

#include <memory>
#include <stdexcept>

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/corpus/document.h"
#include "meta/corpus/metadata_parser.h"
#include "meta/meta.h"
#include "meta/util/optional.h"

namespace meta
{
namespace corpus
{

/**
 * Provides interface to with multiple corpus input formats.
 *
 * Required config parameters:
 * ~~~toml
 * prefix = "prefix"
 * dataset = "datasetname" # relative to prefix
 * corpus = "corpus-spec-file" # e.g. "line.toml"
 * ~~~
 *
 * The corpus spec toml file also requires a corpus type and an optional
 * encoding for the corpus text.
 *
 * Required config parameters:
 * ~~~toml
 * type = "line-corpus" # for example
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * encoding = "utf-8" # default value
 * store-full-text = false # default value; N/A for libsvm-corpus
 * metadata = # metadata schema; see metadata object
 * ~~~
 *
 * @see https://meta-toolkit.org/overview-tutorial.html
 */
class corpus
{
  public:
    /**
     * Constructs a new corpus with the given encoding.
     * @param encoding The encoding to interpret the text as
     */
    corpus(std::string encoding);

    /**
     * @return whether there is another document in this corpus
     */
    virtual bool has_next() const = 0;

    /**
     * @return the next document from this corpus
     */
    virtual document next() = 0;

    /**
     * @return the number of documents in this corpus
     */
    virtual uint64_t size() const = 0;

    /**
     * @return the corpus' metadata schema
     */
    virtual metadata::schema_type schema() const;

    /**
     * Destructor.
     */
    virtual ~corpus() = default;

    /**
     * @return the encoding for the corpus.
     */
    const std::string& encoding() const;

    /**
     * @return whether this corpus will create a metadata field for full text
     * (called "content")
     */
    bool store_full_text() const;

    /**
     * @param store_full_text Tells this corpus to store full document text as
     * metadata
     */
    void set_store_full_text(bool store_full_text);

  protected:
    /**
     * Helper function to be used by deriving classes in implementing
     * next() to set the metadata for the current document.
     */
    std::vector<metadata::field> next_metadata();

  private:
    friend std::unique_ptr<corpus> make_corpus(const cpptoml::table&);

    void set_metadata_parser(metadata_parser&& mdparser);

    /// The type of encoding this document uses
    std::string encoding_;
    /// The metadata parser
    util::optional<metadata_parser> mdata_parser_;
    /// Whether to store the original document text
    bool store_full_text_;
};

/**
 * Basic exception for corpus interactions.
 */
class corpus_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#endif
