/**
 * @file sequence.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SEQUENCE_H_
#define META_SEQUENCE_H_

#include <vector>

#include "meta/config.h"
#include "meta/sequence/observation.h"

namespace meta
{
namespace sequence
{

/**
 * Represents a tagged sequence of observations.
 */
class sequence
{
  public:
    /// the iterator class over the sequence
    using iterator = std::vector<observation>::iterator;
    /// the const_iterator class over the sequence
    using const_iterator = std::vector<observation>::const_iterator;
    /// the type containing the size for the sequence
    using size_type = std::vector<observation>::size_type;

    /**
     * Adds an observation to the end of the sequence.
     * @param obs The observation to add
     */
    void add_observation(observation obs);

    /**
     * Adds a symbol to the end of the sequence, untagged.
     * @param sym The symbol for the untagged observation to add
     */
    void add_symbol(symbol_t sym);

    /**
     * Subscript operator.
     * @param idx The index to access
     * @return a const-ref to the observation at that index in the sequence
     */
    const observation& operator[](size_type idx) const;

    /**
     * Subscript operator.
     * @param idx The index to access
     * @return a reference to the observation at that index in the sequence
     */
    observation& operator[](size_type idx);

    /**
     * @return an iterator to the beginning of the sequence
     */
    iterator begin();

    /**
     * @return an iterator to the end of the sequence
     */
    iterator end();

    /**
     * @return a const_iterator to the beginning of the sequence
     */
    const_iterator begin() const;

    /**
     * @return a const_iterator to the end of the sequence
     */
    const_iterator end() const;

    /**
     * @return the number of observations in the sequence
     */
    size_type size() const;

  private:
    /// the observations in the sequence
    std::vector<observation> observations_;
};
}
}

#endif
