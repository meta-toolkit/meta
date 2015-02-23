/**
 * @file okapi_bm25.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_OKAPI_BM25_H_
#define META_OKAPI_BM25_H_

#include "index/ranker/ranker.h"
#include "index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * The Okapi BM25 scoring function.
 */
class okapi_bm25 : public ranker
{
  public:
    /// The identifier for this ranker.
    const static std::string id;

    /// Default k1, doc term smoothing
    const static constexpr double default_k1 = 1.2;

    /// Default b, length normalization
    const static constexpr double default_b = 0.75;

    /// Default k3, query term smoothing
    const static constexpr double default_k3 = 500.0;

    /**
     * @param k1 Doc term smoothing
     * @param b Length normalization
     * @param k3 Query term smoothing
     */
    okapi_bm25(double k1 = default_k1, double b = default_b,
               double k3 = default_k3);

    /**
     * @param sd score_data for the current query
     */
    double score_one(const score_data& sd) override;

  private:
    /// Doc term smoothing
    const double k1_;
    /// Length normalization
    const double b_;
    /// Query term smoothing
    const double k3_;
};

/**
 * Specialization of the factory method used to create okapi_bm25 rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<okapi_bm25>(const cpptoml::table&);
}
}
#endif
