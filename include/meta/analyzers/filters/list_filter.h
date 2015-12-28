/**
 * @file list_filter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LIST_FILTER_H_
#define META_LIST_FILTER_H_

#include <memory>
#include <unordered_set>

#include "meta/analyzers/filter_factory.h"
#include "meta/util/clonable.h"
#include "meta/util/optional.h"

namespace cpptoml
{
class table;
}

namespace meta
{
namespace analyzers
{
namespace filters
{

/**
 * Filter that either removes or keeps tokens from a given list.
 *
 * Required config parameters:
 * ~~~toml
 * file = "path"
 * ~~~
 * Optional config parameters:
 * ~~~toml
 * type = "accept" # or,
 * type = "reject" # default
 * ~~~
 */
class list_filter : public util::clonable<token_stream, list_filter>
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
     * @param source The source to construct the filter from
     * @param filename A file that lists tokens that should either be accepted
     * or rejected
     * @param method Whether to accept or reject tokens from the list
     */
    list_filter(std::unique_ptr<token_stream> source,
                const std::string& filename, type method = type::REJECT);

    /**
     * Copy constructor.
     * @param other The list_filter to copy into this one
     */
    list_filter(const list_filter& other);

    /**
     * Sets the content for the beginning of the filter chain.
     * @param content The string content to set
     */
    void set_content(std::string&& content) override;

    /**
     * @return the next token in the sequence.
     */
    std::string next() override;

    /**
     * Determines whether there are more tokens available in the stream.
     */
    operator bool() const override;

    /// Identifier for this filter
    const static util::string_view id;

  private:
    /**
     * Advances internal state to the next valid token.
     */
    void next_token();

    /// The source to read tokens from
    std::unique_ptr<token_stream> source_;

    /// The next buffered token
    util::optional<std::string> token_;

    /// The set of tokens used for filtering
    std::unordered_set<std::string> list_;

    /// Whether or not this filter accepts or rejects tokens in the list
    type method_;
};

/**
 * Specialization of the factory method used to create list_filters.
 */
template <>
std::unique_ptr<token_stream>
    make_filter<list_filter>(std::unique_ptr<token_stream>,
                             const cpptoml::table&);
}
}
}
#endif
