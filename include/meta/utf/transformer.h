/**
 * @file transformer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TRANSFORMER_H_
#define META_TRANSFORMER_H_

#include <string>

#include "meta/config.h"
#include "meta/util/pimpl.h"

namespace meta
{
namespace utf
{

/**
 * Class that encapsulates transliteration of unicode strings.
 * @see http://userguide.icu-project.org/transforms
 */
class transformer
{
  public:
    /**
     * Constructs a new transformer.
     *
     * @param id the id of the transliteration rule as defined by ICU
     */
    transformer(const std::string& id);

    /**
     * Copy constructor.
     */
    transformer(const transformer& other);

    /**
     * Move constructor.
     */
    transformer(transformer&& other);

    /**
     * Destructor for the transformer.
     */
    ~transformer();

    /**
     * Transforms the given utf8 string.
     * @return the transformed string, encoded in utf8
     */
    std::string operator()(const std::string& str);

  private:
    /// The implementation class for transformers.
    class impl;
    /// A pointer to the implementation class for this transformer.
    util::pimpl<impl> impl_;
};
}
}
#endif
