/**
 * @file topics/lda_gibbs_inferencer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LDA_GIBBS_INFERENCER_H_
#define META_LDA_GIBBS_INFERENCER_H_

#include "meta/config.h"
#include "meta/stats/multinomial.h"
#include "meta/topics/lda_gibbs.h"
#include "meta/topics/topic_model.h"

namespace meta
{
namespace topics
{

class lda_gibbs::inferencer
{
  public:
    inferencer(const cpptoml::table& config)
    {
        auto topics_cfg = config.get_table("lda");
        if (!topics_cfg)
        {
            throw topic_model_exception{
                "Missing [lda] configuration in configuration file"};
        }

        auto prefix = topics_cfg->get_as<std::string>("model-prefix");
        if (!prefix)
        {
            throw topic_model_exception{
                "Missing prefix key in configuration file"};
        }

        std::ifstream phi{*prefix + ".phi.bin", std::ios::binary};

        if (!phi)
        {
            throw topic_model_exception{"missing topic term probabilities file:"
                                        + *prefix + ".phi.bin"};
        }

        auto alpha = topics_cfg->get_as<double>("alpha");
        if (!alpha)
        {
            throw topic_model_exception{
                "missing alpha parameter in configuration file"};
        }

        auto num_topics = topics_cfg->get_as<uint64_t>("topics");
        if (!num_topics)
        {
            throw topic_model_exception{"missing topics key in [lda] table"};
        }

        prior_ = stats::dirichlet<topic_id>(*alpha, *num_topics);
        load_from_stream(phi);
    }

    inferencer(std::istream& topic_stream, double alpha)
    {
        load_from_stream(topic_stream);
        prior_ = stats::dirichlet<topic_id>(alpha, topics_.size());
    }

    template <class RandomNumberGenerator>
    stats::multinomial<topic_id> operator()(const learn::feature_vector& doc,
                                            std::size_t iters,
                                            RandomNumberGenerator&& rng)
    {
        auto doc_size = std::accumulate(
            doc.begin(), doc.end(), 0.0,
            [](double accum,
               const std::pair<learn::feature_id, double>& weight) {
                return accum + weight.second;
            });

        std::vector<topic_id> assignments(doc_size);
        stats::multinomial<topic_id> proportions{prior_};

        for (std::size_t i = 0; i < iters; ++i)
        {
            detail::sample_document(doc, topics_.size(), assignments,
                                    // decrease counts
                                    [&](topic_id old_topic, term_id) {
                                        if (i > 0)
                                            proportions.decrement(old_topic, 1);
                                    },
                                    // sample weight
                                    [&](topic_id topic, term_id term) {
                                        return proportions.probability(topic)
                                               * topics_[topic].probability(
                                                     term);
                                    },
                                    // increase counts
                                    [&](topic_id new_topic, term_id) {
                                        proportions.increment(new_topic, 1);
                                    },
                                    std::forward<RandomNumberGenerator>(rng));
        }

        return proportions;
    }

  private:
    void load_from_stream(std::istream& topic_stream)
    {
        auto check = [&]() {
            if (!topic_stream)
                throw topic_model_exception{
                    "topic term stream ended unexpectedly"};
        };

        auto num_topics = io::packed::read<std::size_t>(topic_stream);
        check();
        topics_.resize(num_topics);

        io::packed::read<std::size_t>(topic_stream); // discard vocab size
        check();

        printing::progress term_progress{
            " > Loading topic term probabilities: ", num_topics};
        for (topic_id tid{0}; tid < num_topics; ++tid)
        {
            check();
            term_progress(tid);
            io::packed::read(topic_stream, topics_[tid]);
        }
    }

    std::vector<stats::multinomial<term_id>> topics_;
    stats::dirichlet<topic_id> prior_;
};
}
}
#endif
