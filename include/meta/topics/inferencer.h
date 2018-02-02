/**
 * @file topics/inferencer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOPICS_INFERENCER_H_
#define META_TOPICS_INFERENCER_H_

#include "meta/config.h"
#include "meta/topics/topic_model.h"
#include "meta/util/progress.h"

namespace meta
{
namespace topics
{

class inferencer
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

    const stats::multinomial<term_id>& term_distribution(topic_id k) const
    {
        return topics_[k];
    }

    std::size_t num_topics() const
    {
        return topics_.size();
    }

    const stats::dirichlet<topic_id>& proportions_prior() const
    {
        return prior_;
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
