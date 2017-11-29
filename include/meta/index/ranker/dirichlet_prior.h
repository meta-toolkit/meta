/**
 * @file dirichlet_prior.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_DIRICHLET_PRIOR_H_
#define META_DIRICHLET_PRIOR_H_

#include "meta/index/ranker/lm_ranker.h"
#include "meta/index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * Implements Bayesian smoothing with a Dirichlet prior.
 *
 * Required config parameters:
 * ~~~toml
 * [ranker]
 * method = "dirichlet-prior"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * mu = 2000.0
 * ~~~

 */
class dirichlet_prior : public language_model_ranker
{
  public:
    /// Identifier for this ranker.
    const static util::string_view id;

    /// Default value of mu
    const static constexpr float default_mu = 2000.0f;

    /**
     * @param mu
     */
    dirichlet_prior(float mu = default_mu);

    /**
     * Loads a dirichlet_prior ranker from a stream.
     * @param in The stream to read from
     */
    dirichlet_prior(std::istream& in);

    void save(std::ostream& out) const override;

    /**
     * Calculates the smoothed probability of a term.
     * @param sd score_data for the current query
     */
    float smoothed_prob(const score_data& sd) const override;

    /**
     * A document-dependent constant.
     * @param sd score_data for the current query
     */
    float doc_constant(const score_data& sd) const override;

  protected:
    /// the Dirichlet prior parameter
//    const float mu_;
    float mu_;
};

class dirichlet_prior_opt : public dirichlet_prior{
public:
    template <class ForwardIterator>
    std::vector<search_result> score(inverted_index& idx, ForwardIterator begin,
                                     ForwardIterator end,
                                     uint64_t num_results = 10)
    {
        // optimize mu according to ranker_context before ranking
        this->optimize_mu(idx);

        return ranker::score(idx, begin, end, num_results);
    }

    float get_optimized_mu(const inverted_index& idx) {
        optimize(idx);
        return mu_;
    }

private:
    void optimize(const inverted_index& idx) {
        doc_id y = idx.docs()[0];

    }

    virtual void optimize_mu(const inverted_index& idx) = 0;
};

class digamma_rec: public dirichlet_prior_opt{
    void optimize_mu(const inverted_index& idx) override { mu_ = 0;};
};

class log_approx: public dirichlet_prior_opt{
    void optimize_mu(const inverted_index& idx) override { mu_ = 0;};
};

class mackay_peto: public dirichlet_prior_opt{
    void optimize_mu(const inverted_index& idx) override { mu_ = 0;};
};

/**
 * Specialization of the factory method used to create dirichlet_prior
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<dirichlet_prior>(const cpptoml::table&);
}
}
#endif
