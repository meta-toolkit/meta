/**
 * @file observation.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_OBSERVATION_H_
#define META_OBSERVATION_H_

#include <string>
#include <vector>

#include "meta/config.h"
#include "meta/meta.h"
#include "meta/util/identifiers.h"
#include "meta/util/optional.h"

namespace meta
{
namespace sequence
{

MAKE_IDENTIFIER(symbol_t, std::string)
MAKE_IDENTIFIER(tag_t, std::string)
MAKE_IDENTIFIER(feature_id, uint64_t)

/**
 * Represents an observation in a tagged sequence. Contains a symbol and
 * (optionally) a tag for that symbol.
 */
class observation
{
  public:
    /// internal feature vector for observations
    using feature_vector = std::vector<std::pair<feature_id, double>>;

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
     * @throw exception if there is no label
     * @return the label for this observation
     */
    const label_id& label() const;

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
     * Sets the current label.
     * @param t The new label for this observation
     */
    void label(label_id lbl);

    /**
     * @return whether or not this observation is tagged
     */
    bool tagged() const;

    /**
     * @return the feature map for this observation
     */
    const feature_vector& features() const;

    /**
     * @param feats The new feature map for this observation
     */
    void features(feature_vector feats);

  private:
    /// The symbol for this observation
    symbol_t symbol_;
    /// The tag for this observation, if it exists
    util::optional<tag_t> tag_;
    /// The label_id for this observation's tag, if it exists
    util::optional<label_id> label_;
    /// The features for this observation
    feature_vector features_;
};

/**
 * Basic exception class for observation interactions.
 */
class observation_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
#endif
