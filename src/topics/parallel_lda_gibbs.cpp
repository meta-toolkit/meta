/**
 * @file parallel_lda_gibbs.cpp
 * @author Chase Geigle
 */

#include "meta/topics/parallel_lda_gibbs.h"
#include "meta/learn/instance.h"
#include "meta/logging/logger.h"
#include "meta/parallel/parallel_for.h"
#include "meta/util/progress.h"

namespace meta
{
namespace topics
{

void parallel_lda_gibbs::initialize()
{
    for (auto& id : pool_.thread_ids())
        phi_diffs_[id].resize(num_topics_);
    lda_gibbs::initialize();
}

void parallel_lda_gibbs::perform_iteration(uint64_t iter,
                                           bool init /* = false */)
{
    std::string str;
    if (init)
        str = "Initialization: ";
    else
        str = "Iteration " + std::to_string(iter) + ": ";
    printing::progress progress{str, docs_.size()};
    progress.print_endline(false);

    // clear out diffs
    for (auto& phis : phi_diffs_)
        for (auto& phi : phis.second)
            phi.clear();

    std::mutex mutex;
    uint64_t assigned = 0;
    parallel::parallel_for(
        docs_.begin(), docs_.end(), pool_, [&](const learn::instance& doc) {
            {
                std::lock_guard<std::mutex> lock{mutex};
                progress(assigned++);
            }
            size_t n = 0; // term number within document---constructed
                          // so that each occurrence of the same term
                          // can still be assigned a different topic
            for (const auto& freq : doc.weights)
            {
                for (size_t j = 0; j < freq.second; ++j)
                {
                    auto old_topic = doc_word_topic_[doc.id][n];
                    // don't include current topic assignment in
                    // probability calculation
                    if (!init)
                        decrease_counts(old_topic, freq.first, doc.id);

                    // sample a new topic assignment
                    auto topic = sample_topic(freq.first, doc.id);
                    doc_word_topic_[doc.id][n] = topic;

                    // increase counts
                    increase_counts(topic, freq.first, doc.id);
                    n += 1;
                }
            }
        });

    // reduce down the distribution diffs for phi into the global
    // distributions for phi
    for (const auto& phis_pair : phi_diffs_)
    {
        const auto& phis = phis_pair.second;
        for (topic_id topic{0}; topic < phis.size(); ++topic)
            phi_[topic] += phis[topic];
    }
}

void parallel_lda_gibbs::decrease_counts(topic_id topic, term_id term,
                                         learn::instance_id doc)
{
    auto tid = std::this_thread::get_id();
    phi_diffs_[tid][topic].decrement(term, 1);
    theta_[doc].decrement(topic, 1);
}

void parallel_lda_gibbs::increase_counts(topic_id topic, term_id term,
                                         learn::instance_id doc)
{
    auto tid = std::this_thread::get_id();
    phi_diffs_[tid][topic].increment(term, 1);
    theta_[doc].increment(topic, 1);
}

double parallel_lda_gibbs::compute_sampling_weight(term_id term,
                                                   learn::instance_id doc,
                                                   topic_id topic) const
{
    auto tid = std::this_thread::get_id();
    return (phi_[topic].counts(term) + phi_diffs_.at(tid)[topic].counts(term))
           / (phi_[topic].counts() + phi_diffs_.at(tid)[topic].counts())
           * compute_doc_topic_probability(doc, topic);
}
}
}
