/**
 * @file multinomial.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_STATS_MULTINOMIAL_H_
#define META_STATS_MULTINOMIAL_H_

#include <cstdint>
#include <random>
#include "stats/dirichlet.h"
#include "util/sparse_vector.h"

namespace meta
{
namespace stats
{

/**
 * Represents a multinomial/categorical distribution.
 */
template <class T>
class multinomial
{
  public:
    /**
     * Creates a multinomial distribution. No events or probabilities are
     * initialized.
     */
    multinomial();

    /**
     * Creates a multinomial distribution with a Dirichlet prior
     * distribution over the parameters.
     *
     * @param prior The prior distribution
     */
    multinomial(dirichlet<T> prior);

    /**
     * Observes an event and adjusts the distribution's probabilities
     * accordingly.
     *
     * @param event The event observed
     * @param count The number of times it was observed
     */
    void increment(const T& event, uint64_t count);

    /**
     * Removes observations of an event and adjusts the distribution's
     * probabilities accordingly.
     *
     * @param event The event
     * @param count The number of counts of this event to remove
     */
    void decrement(const T& event, uint64_t count);

    /**
     * Removes all observations.
     */
    void clear();

    /**
     * Obtains the probability of an event.
     */
    double probability(const T& event) const;

    /**
     * Samples from the distribution.
     * @param gen The random number generator to be used
     */
    template <class Generator>
    const T& operator()(Generator&& gen) const;

  private:
    util::sparse_vector<T, uint64_t> counts_;
    uint64_t total_counts_;
    dirichlet<T> prior_;
};

}
}
#include "stats/multinomial.tcc"
#endif
