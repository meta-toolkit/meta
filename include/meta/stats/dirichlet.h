/**
 * @file dirichlet.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_STATS_DIRICHLET_H_
#define META_STATS_DIRICHLET_H_

#include <cstdint>
#include <memory>

#include "meta/config.h"
#include "meta/io/packed.h"
#include "meta/util/sparse_vector.h"

namespace meta
{
namespace stats
{

/**
 * Represents a Dirichlet distribution. Typically used as a prior for
 * multinomials in the language modeling, topic modeling, classification,
 * and sequence labeling components.
 */
template <class T>
class dirichlet
{
  public:
    /**
     * Constructs an empty (0, 0) Dirichlet.
     */
    dirichlet();

    /**
     * Constructs a symmetric Dirichlet with concentration parameter
     * \f$\alpha\f$ and dimension \f$n\f$.
     *
     * @param alpha The concentration parameter
     * @param n The dimensionality of the Dirichlet
     */
    dirichlet(double alpha, uint64_t n);

    /**
     * Constructs a Dirichlet from a sequence of parameters that make up
     * the parameter vector \f$\mathbf{\alpha}\f$. Each element is expected
     * to be a std::pair<T, double>.
     *
     * @param begin An iterator to the beginning of the sequence
     * @param end An iterator to the ending of the sequence
     */
    template <class Iter>
    dirichlet(Iter begin, Iter end);

    /**
     * Copy constructor.
     */
    dirichlet(const dirichlet& other);

    /**
     * Move constructor.
     */
    dirichlet(dirichlet&& other);

    /**
     * Assignment operator.
     */
    dirichlet& operator=(dirichlet rhs);

    /**
     * Destructor.
     */
    ~dirichlet();

    /**
     * Obtains the number of "pseudo-counts" associated with a given event
     * when used as a prior for a multinomial distribution.
     *
     * @param event The event in question
     * @return the hyperparameter for the given event
     */
    double pseudo_counts(const T& event) const;

    /**
     * Obtains the number of total "pseudo-counts" associated with this
     * distribution when used as a prior for a multinomial distribution.
     */
    double pseudo_counts() const;

    /**
     * Swaps this dirichlet with the parameter.
     * @param other The dirichlet to swap with
     */
    void swap(dirichlet& other);

    /**
     * Writes the dirichlet to a stream.
     * @param out The stream to write to
     */
    void save(std::ostream& out) const;

    /**
     * Reads the dirichlet from a stream.
     * @param in The stream to read from
     */
    void load(std::istream& in);

    template <class OutputStream>
    friend uint64_t packed_write(OutputStream& os, const dirichlet& dist)
    {
        auto bytes = io::packed::write(os, static_cast<uint64_t>(dist.type_));
        switch (dist.type_)
        {
            case type::SYMMETRIC:
            {
                bytes += io::packed::write(os, dist.params_.fixed_alpha_);
                bytes += io::packed::write(
                    os, static_cast<uint64_t>(dist.alpha_sum_
                                              / dist.params_.fixed_alpha_));
                break;
            }
            case type::ASYMMETRIC:
            {
                bytes += io::packed::write(os, dist.params_.sparse_alpha_);
                break;
            }
        }
        return bytes;
    }

    template <class InputStream>
    friend uint64_t packed_read(InputStream& is, dirichlet& dist)
    {
        uint64_t typ;
        auto bytes = io::packed::read(is, typ);
        if (bytes == 0)
            return 0;

        type read_type = static_cast<type>(typ);
        switch (read_type)
        {
            case type::SYMMETRIC:
            {
                double alpha;
                bytes += io::packed::read(is, alpha);
                uint64_t n;
                bytes += io::packed::read(is, n);
                dist = dirichlet{alpha, n};
                break;
            }
            case type::ASYMMETRIC:
            {
                std::vector<std::pair<T, double>> vec;
                bytes += io::packed::read(is, vec);
                dist = dirichlet{vec.begin(), vec.end()};
                break;
            }
        }
        return bytes;
    }

  private:
    enum class type
    {
        SYMMETRIC,
        ASYMMETRIC
    } type_;

    union parameters {
        parameters()
        {
            // nothing
        }

        parameters(double alpha)
        {
            fixed_alpha_ = alpha;
        }

        template <class Iter>
        parameters(Iter begin, Iter end)
        {
            new (&sparse_alpha_) util::sparse_vector<T, double>{begin, end};
        }

        ~parameters()
        {
            // nothing
        }

        double fixed_alpha_;
        util::sparse_vector<T, double> sparse_alpha_;
    } params_;

    double alpha_sum_;
};
}
}

#include "meta/stats/dirichlet.tcc"
#endif
