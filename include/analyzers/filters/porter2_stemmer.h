/**
 * @file porter2_stemmer.h
 * @author Chase Geigle
 */

#ifndef _META_FILTER_PORTER2_STEMMER_H_
#define _META_FILTER_PORTER2_STEMMER_H_

#include <memory>
#include "analyzers/token_stream.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
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
     */
    porter2_stemmer(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     */
    porter2_stemmer(const porter2_stemmer& other);

    /**
     * Sets the content for the beginning of the filter chain.
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

    /**
     * Identifier for this filter.
     */
    const static std::string id;

  private:
    /**
     * The stream to read tokens from.
     */
    std::unique_ptr<token_stream> source_;
};
}
}
#endif
