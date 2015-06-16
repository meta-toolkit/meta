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

#include <stdexcept>
#include <memory>

#include "meta.h"
#include "corpus/document.h"
#include "corpus/metadata_parser.h"
#include "util/optional.h"

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
 * Optional config parameters: none.
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
    virtual metadata::schema schema() const;

    /**
     * Destructor.
     */
    virtual ~corpus() = default;

    /**
     * @return the encoding for the corpus.
     */
    const std::string& encoding() const;

    /**
     * @param config_file The cpptoml config file containing what type of
     * corpus to load
     * @return a unique_ptr to the corpus object containing the documents
     */
    static std::unique_ptr<corpus> load(const std::string& config_file);

    /**
     * Basic exception for corpus interactions.
     */
    class corpus_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  protected:
    /**
     * Helper function to be used by deriving classes in implementing
     * next() to set the metadata for the current document.
     */
    std::vector<metadata::field> next_metadata();

  private:
    void set_metadata_parser(metadata_parser&& mdparser);

    /// The type of encoding this document uses
    std::string encoding_;
    /// The metadata parser
    util::optional<metadata_parser> mdata_parser_;
};
}
}

#endif
