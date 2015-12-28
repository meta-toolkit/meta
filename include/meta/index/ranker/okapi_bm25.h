/**
 * @file okapi_bm25.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_OKAPI_BM25_H_
#define META_OKAPI_BM25_H_

#include "meta/index/ranker/ranker.h"
#include "meta/index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * The Okapi BM25 scoring function.
 *
 * Required config parameters:
 * ~~~toml
 * [ranker]
 * method = "bm25"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * k1 = 1.2
 * b = 0.75
 * k3 = 500.0
 * ~~~
 */
class okapi_bm25 : public ranker
{
  public:
    /// The identifier for this ranker.
    const static util::string_view id;

    /// Default k1, doc term smoothing
    const static constexpr float default_k1 = 1.2f;

    /// Default b, length normalization
    const static constexpr float default_b = 0.75f;

    /// Default k3, query term smoothing
    const static constexpr float default_k3 = 500.0f;

    /**
     * @param k1 Doc term smoothing
     * @param b Length normalization
     * @param k3 Query term smoothing
     */
    okapi_bm25(float k1 = default_k1, float b = default_b,
               float k3 = default_k3);

    /**
     * Loads an okapi_bm25 ranker from a stream.
     * @param in The stream to read from
     */
    okapi_bm25(std::istream& in);

    /**
     * @param sd score_data for the current query
     */
    float score_one(const score_data& sd) override;

    void save(std::ostream& out) const override;

  private:
    /// Doc term smoothing
    const float k1_;
    /// Length normalization
    const float b_;
    /// Query term smoothing
    const float k3_;
};

/**
 * Specialization of the factory method used to create okapi_bm25 rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<okapi_bm25>(const cpptoml::table&);
}
}
#endif
