/**
 * @file list_filter.h
 * @author Chase Geigle
 */

#ifndef _META_LIST_FILTER_H_
#define _META_LIST_FILTER_H_

#include <memory>
#include <unordered_set>

#include "analyzers/token_stream.h"
#include "util/optional.h"

namespace meta
{
namespace analyzers
{

class list_filter : public token_stream
{
  public:

    /**
     * Strongly typed flag to indicate whether the list_filter rejects
     * tokens in the list or only accepts tokens in the list.
     */
    enum class type
    {
        ACCEPT,
        REJECT
    };

    /**
     * Creates a list_filter reading tokens from the given source and
     * filtering based on the tokens specified in the given file. The
     * method by default is to reject any tokens specified in the file, but
     * it may be optionally set to type::ACCEPT to allow only tokens that
     * appear in that list.
     */
    list_filter(std::unique_ptr<token_stream> source,
                const std::string& filename,
                type method = type::REJECT);

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
    /**
     * Advances internal state to the next valid token.
     */
    void next_token();

    /**
     * The source to read tokens from.
     */
    std::unique_ptr<token_stream> source_;

    /**
     * The next buffered token.
     */
    util::optional<std::string> token_;

    /**
     * The set of tokens used for filtering.
     */
    std::unordered_set<std::string> list_;

    /**
     * Whether or not this filter accepts or rejects tokens in the list.
     */
    type method_;
};
}
}
#endif
