/**
 * @file transformer.h
 * @author Chase Geigle
 */

#ifndef META_UTIL_TRANSFORMER_H_
#define META_UTIL_TRANSFORMER_H_

#include <string>

#include "util/pimpl.h"

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
     */
    std::string operator()(const std::string& str);

  private:
    class impl;
    util::pimpl<impl> impl_;
};
}
}
#endif
