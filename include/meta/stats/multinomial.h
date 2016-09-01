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

#include "meta/config.h"
#include "meta/stats/dirichlet.h"
#include "meta/util/sparse_vector.h"

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
     * The event type for this distribution.
     */
    using event_type = T;

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
    void increment(const T& event, double count);

    /**
     * Removes observations of an event and adjusts the distribution's
     * probabilities accordingly.
     *
     * @param event The event
     * @param count The number of counts of this event to remove
     */
    void decrement(const T& event, double count);

    /**
     * @param event The event
     * @return the number of observations (including the prior) for this
     * event
     */
    double counts(const T& event) const;

    /**
     * @return the total number of observations (including the prior)
     */
    double counts() const;

    /**
     * @return the number of unique event values that have been observed
     */
    uint64_t unique_events() const;

    /**
     * Runs a function for each observed event for this distribution. Note
     * that this does **not** include the prior, only events that have been
     * explicitly observed with e.g. the increment() function.
     *
     * @param fun The function to run for each seen event for this
     * distribution
     */
    template <class Fun>
    void each_seen_event(Fun&& fun) const;

    /**
     * Removes all observations.
     */
    void clear();

    /**
     * Obtains the probability of an event.
     */
    double probability(const T& event) const;

    /**
     * @return the prior
     */
    const dirichlet<T>& prior() const;

    /**
     * Samples from the distribution.
     * @param gen The random number generator to be used
     */
    template <class Generator>
    const T& operator()(Generator&& gen) const;

    /**
     * Adds in the observations of another multinomial to this one.
     *
     * @param other The other multinomial to merge with.
     */
    multinomial<T>& operator+=(const multinomial<T>& other);

    /**
     * Saves the distribution to a stream.
     * @param out The stream to write to
     */
    void save(std::ostream& out) const;

    /**
     * Reads the distribution from a stream.
     * @param in The stream to read from
     */
    void load(std::istream& in);

  private:
    util::sparse_vector<T, double> counts_;
    double total_counts_;
    dirichlet<T> prior_;
};

template <class T>
multinomial<T> operator+(const multinomial<T>& lhs, const multinomial<T>& rhs)
{
    multinomial<T> res{lhs};
    res += rhs;
    return res;
}

template <class T>
multinomial<T> operator+(multinomial<T>&& lhs, const multinomial<T>& rhs)
{
    return lhs += rhs;
}

template <class T>
multinomial<T> operator+(const multinomial<T>& lhs, multinomial<T>&& rhs)
{
    return rhs += lhs;
}

template <class T>
multinomial<T> operator+(multinomial<T>&& lhs, multinomial<T>&& rhs)
{
    return lhs += rhs;
}
}
}
#include "meta/stats/multinomial.tcc"
#endif
