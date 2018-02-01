/**
 * @file parallel_lda_gibbs.cpp
 * @author Chase Geigle
 */

#include "meta/topics/parallel_lda_gibbs.h"
#include "meta/learn/instance.h"
#include "meta/logging/logger.h"
#include "meta/parallel/algorithm.h"
#include "meta/util/progress.h"

namespace meta
{
namespace topics
{
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

    std::mutex mutex;
    uint64_t assigned = 0;

    // compute a vector of differences to apply to each topic in parallel
    // across all of the threads in the pool
    auto phi_diffs = parallel::reduction(
        docs_.begin(), docs_.end(), pool_,
        // local storage
        [&]() { return std::vector<stats::multinomial<term_id>>(num_topics_); },
        // map
        [&](std::vector<stats::multinomial<term_id>>& diffs,
            const learn::instance& doc) {
            {
                std::lock_guard<std::mutex> lock{mutex};
                progress(assigned++);
            }

            detail::sample_document(
                doc.weights, num_topics_, doc_word_topic_[doc.id],
                // decrease counts
                [&](topic_id old_topic, term_id term) {
                    if (!init)
                    {
                        theta_[doc.id].decrement(old_topic, 1);
                        diffs[old_topic].decrement(term, 1);
                    }
                },
                // compute sampling weight
                [&](topic_id topic, term_id term) {
                    return theta_[doc.id].probability(topic)
                           * (phi_[topic].counts(term)
                              + diffs[topic].counts(term))
                           / (phi_[topic].counts() + diffs[topic].counts());
                },
                // increase counts
                [&](topic_id new_topic, term_id term) {
                    theta_[doc.id].increment(new_topic, 1);
                    diffs[new_topic].increment(term, 1);
                },
                rng_);
        },
        // reduce
        [&](std::vector<stats::multinomial<term_id>>& diffs_total,
            const std::vector<stats::multinomial<term_id>>& diffs) {
            for (topic_id topic{0}; topic < num_topics_; ++topic)
                diffs_total[topic] += diffs[topic];
        });

    // incorporate the reduced diffs from all of the threads
    for (topic_id topic{0}; topic < num_topics_; ++topic)
        phi_[topic] += phi_diffs[topic];
}
}
}
