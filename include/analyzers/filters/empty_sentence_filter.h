/**
 * @file empty_sentence_filter.h
 * @author Chase Geigle
 */

#ifndef _META_EMPTY_SENTENCE_FILTER_H_
#define _META_EMPTY_SENTENCE_FILTER_H_

#include "analyzers/token_stream.h"
#include "util/clonable.h"
#include "util/optional.h"

namespace meta
{
namespace analyzers
{
/**
 * Filter that removes any empty sentences from the token stream. Empty
 * sentences can be caused by filters in the filter chain that follow
 * sentence boundary detection.
 */
class empty_sentence_filter
    : public util::clonable<token_stream, empty_sentence_filter>
{
  public:
    /**
     * Constructs an empty_sentence_filter which reads tokens from the
     * given source.
     */
    empty_sentence_filter(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     */
    empty_sentence_filter(const empty_sentence_filter& other);

    /**
     * Sets the content for the beginning of the filter chain.
     */
    void set_content(const std::string& content) override;

    /**
     * Obtains the next token in the sequence.
     */
    std::string next() override;

    /**
     * Determines whether there are more tokens available in the stream.
     */
    operator bool() const override;
  private:
    void next_token();

    /**
     * The source to read tokens from.
     */
    std::unique_ptr<token_stream> source_;

    util::optional<std::string> first_;
    util::optional<std::string> second_;
};
}
}

#endif
