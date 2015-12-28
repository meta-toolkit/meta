/**
 * @file icu_filter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_ICU_FILTER_H_
#define META_ICU_FILTER_H_

#include "meta/analyzers/filter_factory.h"
#include "meta/util/clonable.h"
#include "meta/util/optional.h"
#include "meta/utf/transformer.h"

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
 * Filter that applies an ICU transliteration to each token in the
 * sequence.
 *
 * Required config parameters:
 * ~~~toml
 * id = "transformer"
 * ~~~
 *
 * Optional config parameters: none.
 *
 * @see http://userguide.icu-project.org/transforms/general/rules
 */
class icu_filter : public util::clonable<token_stream, icu_filter>
{
  public:
    /**
     * Constructs an icu_filter which reads tokens from the given source,
     * using a utf::transformer constructed with the specified id.
     * @param source Where to read tokens from
     * @param id To specify which utf::transformer to use
     */
    icu_filter(std::unique_ptr<token_stream> source, const std::string& id);

    /**
     * Copies an icu_filter.
     * @param other The other filter to copy from
     */
    icu_filter(const icu_filter& other);

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
     * Finds the next valid token for this filter.
     */
    void next_token();

    /// The source to read tokens from
    std::unique_ptr<token_stream> source_;

    /// The transformer to use
    utf::transformer trans_;

    /// Current token (if available)
    util::optional<std::string> token_;
};

/**
 * Specialization of the factory method for creating icu_filters.
 */
template <>
std::unique_ptr<token_stream>
    make_filter<icu_filter>(std::unique_ptr<token_stream>,
                            const cpptoml::table&);
}
}
}
#endif
