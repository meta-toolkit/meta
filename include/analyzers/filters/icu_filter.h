/**
 * @file icu_filter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_ICU_FILTER_H_
#define _META_ICU_FILTER_H_

#include "analyzers/filter_factory.h"
#include "util/clonable.h"
#include "util/optional.h"
#include "utf/transformer.h"

namespace cpptoml {
class toml_group;
}

namespace meta
{
namespace analyzers
{

/**
 * Filter that applies an ICU transliteration to each token in the
 * sequence.
 */
class icu_filter : public util::clonable<token_stream, icu_filter>
{
  public:
    /**
     * Constructs an icu_filter which reads tokens from the given source,
     * using a utf::transformer constructed with the specified id.
     */
    icu_filter(std::unique_ptr<token_stream> source, const std::string& id);

    /**
     * Copies an icu_filter.
     */
    icu_filter(const icu_filter& other);

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

    /**
     * Identifier for this filter.
     */
    const static std::string id;

  private:
    void next_token();

    std::unique_ptr<token_stream> source_;
    utf::transformer trans_;
    util::optional<std::string> token_;
};

/**
 * Specialization of the factory method for creating icu_filters.
 */
template <>
std::unique_ptr<token_stream>
    make_filter<icu_filter>(std::unique_ptr<token_stream>,
                            const cpptoml::toml_group&);
}
}
#endif
