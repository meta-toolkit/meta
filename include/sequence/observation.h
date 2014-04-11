/**
 * @file sequence.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_OBSERVATION_H_
#define META_OBSERVATION_H_

#include <string>
#include <unordered_map>

#include "util/identifiers.h"
#include "util/optional.h"

namespace meta
{
namespace sequence
{

MAKE_IDENTIFIER(symbol_t, std::string)
MAKE_IDENTIFIER(tag_t, std::string)

/**
 * Represents an observation in a tagged sequence. Contains a symbol and
 * (optionally) a tag for that symbol.
 */
class observation
{
  public:
    /**
     * Constructs an observation with a tag.
     * @param sym The symbol for the observation
     * @param t The tag for the observation
     */
    observation(symbol_t sym, tag_t t);

    /**
     * Constructs an observation that does not yet have a tag.
     * @param sym The symbol for the observation
     */
    observation(symbol_t sym);

    /**
     * @return the symbol for this observation
     */
    const symbol_t& symbol() const;

    /**
     * @throw exception if there is no tag
     * @return the tag for this observation
     */
    const tag_t& tag() const;

    /**
     * Sets the current symbol
     * @param sym The new symbol for this observation
     */
    void symbol(symbol_t sym);

    /**
     * Sets the current tag
     * @param t The new tag for this observation
     */
    void tag(tag_t t);

    /**
     * @return whether or not this observation is tagged
     */
    bool tagged() const;

    /**
     * Increment the count for the specified feature.
     * @param feature The feature to increment
     * @param amount The amount to increment by
     */
    void increment(const std::string& feature, double amount);

    /**
     * @return the feature map for this observation
     */
    const std::unordered_map<std::string, double>& features() const;

    /**
     * Basic exception class for observation interactions.
     */
    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  private:
    /// The symbol for this observation
    symbol_t symbol_;
    /// The tag for this observation, if it exists
    util::optional<tag_t> tag_;
    /// The features for this observation
    std::unordered_map<std::string, double> features_;
};
}
}
#endif
