/**
 * @file jelinek_mercer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_JELINEK_MERCER_H_
#define META_JELINEK_MERCER_H_

#include "meta/index/ranker/lm_ranker.h"
#include "meta/index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * Implements the Jelinek-Mercer smoothed ranking model. This smoothing method
 * can be viewed as a linear interpolation between the query term probablity
 * and the collection term probability. The model parameter lambda is the
 * weighting of this interpolation.
 *
 * Required config parameters:
 * ~~~toml
 * [ranker]
 * method = "jelinek-mercer"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * lambda = 0.7
 * ~~~
 */
class jelinek_mercer : public language_model_ranker
{
  public:
    /// The identifier for this ranker.
    const static util::string_view id;

    /// Default value of lambda
    const static constexpr float default_lambda = 0.7f;

    /**
     * @param lambda
     */
    jelinek_mercer(float lambda = default_lambda);

    /**
     * Loads a jelinek_mercer ranker from a stream.
     * @param in The stream to read from
     */
    jelinek_mercer(std::istream& in);

    void save(std::ostream& out) const override;

    /**
     * Calculates the smoothed probability of a term.
     * @param sd
     */
    float smoothed_prob(const score_data& sd) const override;

    /**
     * A document-dependent constant.
     * @param sd
     */
    float doc_constant(const score_data& sd) const override;

  private:
    /// the JM parameter
    const float lambda_;
};

/**
 * Specialization of the factory method used to create jelinek_mercer
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<jelinek_mercer>(const cpptoml::table&);
}
}

#endif
