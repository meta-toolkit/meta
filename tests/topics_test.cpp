/**
 * @file topics_test.cpp
 * @author Sean Massung
 */

#include <fstream>

#include "bandit/bandit.h"
#include "create_config.h"
#include "meta/index/forward_index.h"
#include "meta/learn/dataset.h"
#include "meta/topics/lda_cvb.h"
#include "meta/topics/lda_gibbs.h"
#include "meta/topics/lda_scvb.h"
#include "meta/topics/parallel_lda_gibbs.h"
#include "meta/topics/topic_model.h"

using namespace bandit;
using namespace meta;

namespace
{

template <class TopicModel>
void run_model(const learn::dataset& docs, const std::string& prefix)
{
    {
        const double delta = 0.0000001;
        const uint64_t num_topics = 3;
        TopicModel model{docs, num_topics, 0.1, 0.1}; // alpha = beta = 0.1
        AssertThat(model.num_topics(), Equals(num_topics));
        model.run(3); // only run for three iterations

        // all term probs for all topics should sum to 1
        for (topic_id topic{0}; topic < model.num_topics(); ++topic)
        {
            double sum = 0.0;
            for (term_id term{0}; term < docs.total_features(); ++term)
            {
                sum += model.compute_term_topic_probability(term, topic);
            }
            AssertThat(sum, EqualsWithDelta(1.0, delta));
        }

        // all topic probs for all docs should sum to 1
        for (const auto& doc : docs)
        {
            double sum = 0.0;
            for (topic_id topic{0}; topic < model.num_topics(); ++topic)
                sum += model.compute_doc_topic_probability(doc.id, topic);
            AssertThat(sum, EqualsWithDelta(1.0, delta));
        }
        model.save(prefix);

        std::ifstream theta{prefix + ".theta.bin", std::ios::binary};
        std::ifstream phi{prefix + ".phi.bin", std::ios::binary};
        topics::topic_model t_model{theta, phi};

        AssertThat(t_model.num_words(), Equals(docs.total_features()));
        AssertThat(t_model.num_topics(), Equals(model.num_topics()));

        // the term distributions for each topic should equal those from
        // the current learner state and should sum to 1
        for (topic_id k{0}; k < t_model.num_topics(); ++k)
        {
            const auto& dist = t_model.term_distribution(k);
            double sum = 0.0;
            for (term_id w{0}; w < t_model.num_words(); ++w)
            {
                auto t_prob = dist.probability(w);
                auto m_prob = model.compute_term_topic_probability(w, k);
                AssertThat(t_prob, EqualsWithDelta(m_prob, delta));
                sum += t_prob;
            }
            AssertThat(sum, EqualsWithDelta(1.0, delta));
        }

        // the topic distributions for each document should equal those
        // from the current learner state and should sum to 1
        for (doc_id d{0}; d < t_model.num_docs(); ++d)
        {
            const auto& dist = t_model.topic_distribution(d);
            double sum = 0.0;
            for (topic_id k{0}; k < t_model.num_topics(); ++k)
            {
                auto t_prob = dist.probability(k);
                auto m_prob = model.compute_doc_topic_probability(
                    learn::instance_id(d), k);
                AssertThat(t_prob, EqualsWithDelta(m_prob, delta));
                sum += t_prob;
            }
            AssertThat(sum, EqualsWithDelta(1.0, delta));
        }
    }

    AssertThat(filesystem::file_exists(prefix + ".phi.bin"), IsTrue());
    AssertThat(filesystem::file_exists(prefix + ".theta.bin"), IsTrue());
    filesystem::delete_file(prefix + ".phi");
    filesystem::delete_file(prefix + ".theta");
}
}

go_bandit([]() {

    describe("[topics]", [&]() {
        const std::string prefix = "meta-test-lda-model";
        auto config = tests::create_config("line");
        auto idx = index::make_index<index::forward_index>(*config);
        auto doc_list = idx->docs();
        learn::dataset docs{idx, doc_list.begin(), doc_list.end()};

        it("should run LDA with CVB inference",
           [&]() { run_model<topics::lda_cvb>(docs, prefix); });

        it("should run LDA with Gibbs sampling inference",
           [&]() { run_model<topics::lda_gibbs>(docs, prefix); });

        it("should run LDA with SCVB0 inference",
           [&]() { run_model<topics::lda_scvb>(docs, prefix); });

        it("should run LDA with parallel Gibbs inference",
           [&]() { run_model<topics::parallel_lda_gibbs>(docs, prefix); });
    });

    filesystem::remove_all("ceeaus");
});
