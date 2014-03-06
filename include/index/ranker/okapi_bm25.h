/**
 * @file okapi_bm25.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _OKAPI_BM25_H_
#define _OKAPI_BM25_H_

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
    /**
     * The identifier for this ranker.
     */
    const static std::string id;

    const static constexpr double default_k1 = 1.2;
    const static constexpr double default_b = 0.75;
    const static constexpr double default_k3 = 500.0;

    /**
     * @param k1
     * @param b
     * @param k3
     */
    okapi_bm25(double k1 = default_k1, double b = default_b,
               double k3 = default_k3);

    /**
     * @param sd
     */
    double score_one(const score_data& sd) override;

  private:
    const double _k1;
    const double _b;
    const double _k3;
};

/**
 * Specialization of the factory method used to create okapi_bm25 rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<okapi_bm25>(const cpptoml::toml_group&);
}
}
#endif
