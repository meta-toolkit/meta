/**
 * @file rank_correlation.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_RANK_CORRELATION_H_
#define META_INDEX_RANK_CORRELATION_H_

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace meta
{
namespace index
{

/**
 * Exception thrown when interacting with rank_correlation instances.
 */
class rank_correlation_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * Evaluates two different lists of ranks for correlation with various
 * measures. This class calculates counts of pairs \f$(x_i, y_i)\f$,
 * \f$(x_j, y_j)\f$ that have certain properties:
 *
 * - a pair is called *concordant* if either \f$x_i < x_j\f$ and
 *   \f$y_i < y_j\f$ or \f$x_i > x_j\f$ and \f$y_i > y_j\f$.
 * - a pair is called *discordant* if either \f$x_i < x_j\f$ and
 *   \f$y_i > y_j\f$ or \f$x_i > x_j\f$ and \f$y_i < y_j\f$.
 */
class rank_correlation
{
  public:
    using exception = rank_correlation_exception;

    /**
     * Computes the needed statistics for computing rank correlation
     * metrics. The list x and the list y are assumed to be the same length
     * and x[i] and y[i] give the rank of item i in the two ranked lists to
     * be compared.
     *
     * Ties are allowed, but **be careful in selecting the correct metrics
     * if you have ties**.
     *
     * @param x The ranks for the items in the first ranked list
     * @param y The ranks for the items in the second ranked list
     */
    rank_correlation(const std::vector<std::size_t>& x,
                     const std::vector<std::size_t>& y);

    /**
     * Goodman and Kruskal's gamma. This is an acceptable coefficient even
     * in the presence of ties.
     *
     * @see https://en.wikipedia.org/wiki/Goodman_and_Kruskal's_gamma
     *
     * Let nc be the number of concordant pairs and nd be the number of
     * discordant pairs.
     *
     * @return (nc - nd) / (nc + nd)
     */
    double gamma() const;

    /**
     * Kendall's tau rank correlation coefficient. This is an acceptable
     * coefficient if there are no ties in either ranking.
     *
     * @see http://www.r-tutor.com/gpu-computing/correlation/kendall-rank-coefficient
     *
     * Let nc be the number of concordant pairs and nd be the number of
     * discordant pairs.
     *
     * @return (nc - nd) / (n * (n-1) / 2)
     */
    double tau_a() const;

    /**
     * An adjusted Kendall's tau rank correlation coefficient to account
     * for ties in either ranking.
     *
     * @see http://www.r-tutor.com/gpu-computing/correlation/kendall-tau-b
     *
     * Let nc be the number of concordant pairs and nd be the number of
     * discordant pairs. Let tx be the number of pairs with ties in x and
     * ty be the number of pairs with ties in y. Note that if there is a
     * tie in both x and y, neither tx nor ty is incremented.
     *
     * @return (nc - nd) / sqrt((nc + nd + tx) * (nc + nd + ty))
     */
    double tau_b() const;

  private:
    double nc() const
    {
        return static_cast<double>(num_concordant_);
    }

    double nd() const
    {
        return static_cast<double>(num_discordant_);
    }

    uint64_t num_concordant_;
    uint64_t num_discordant_;
    uint64_t num_ties_x_;
    uint64_t num_ties_y_;
    uint64_t n_;
};
}
}
#endif
