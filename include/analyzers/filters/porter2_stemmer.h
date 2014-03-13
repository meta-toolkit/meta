/**
 * @file porter2_stemmer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_FILTER_PORTER2_STEMMER_H_
#define META_FILTER_PORTER2_STEMMER_H_

#include <memory>
#include "analyzers/token_stream.h"
#include "util/clonable.h"
#include "util/optional.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

/**
 * Filter that stems words according to the porter2 stemmer algorithm.
 * Requires that the porter2 stemmer project submodule be downloaded.
 */
class porter2_stemmer : public util::clonable<token_stream, porter2_stemmer>
{
  public:
    /**
     * Constructs a new porter2 stemmer filter, reading tokens from
     * the given source.
     * @param source The source to construct the filter from
     */
    porter2_stemmer(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     * @param other The porter2_stemmer to copy into this one
     */
    porter2_stemmer(const porter2_stemmer& other);

    /**
     * Sets the content for the beginning of the filter chain.
     * @param content The string content to set
     */
    void set_content(const std::string& content) override;

    /**
     * Obtains the next token in the sequence.
     */
    std::string next() override;

    /**
     * Determines if there are more tokens available in the stream.
     */
    operator bool() const override;

    /// Identifier for this filter
    const static std::string id;

  private:
    /**
     * Finds the next valid token for this filter.
     */
    void next_token();

    /// The stream to read tokens from
    std::unique_ptr<token_stream> source_;

    /// The buffered next token.
    util::optional<std::string> token_;
};
}
}
}
#endif
