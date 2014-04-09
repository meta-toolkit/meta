/**
 * @file batch_training.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_BATCH_TRAINING_H_
#define META_BATCH_TRAINING_H_

#include <algorithm>
#include <random>
#include <vector>

#include "logging/logger.h"
#include "meta.h"

namespace meta
{
namespace classify
{

/**
 * This trains a classifier in an online fashion, using batches of size
 * batch_size from the training_set.
 *
 * @param idx The index the classifier is using (so the cache may be
 * dropped between batches)
 * @param cls The classifier to train. This must be a classifier
 * supporting online learning (e.g., sgd or an ensemble of sgd)
 * @param training_set The list of document ids that comprise the training
 * data
 * @param batch_size The size of the batches to use for the minibatch
 * training
 */
template <class Index, class Classifier>
void batch_train(Index& idx, Classifier& cls,
                 const std::vector<doc_id>& training_set, uint64_t batch_size)
{
    auto docs = training_set;
    std::mt19937 gen(std::random_device{}());
    std::shuffle(docs.begin(), docs.end(), gen);

    // integer-math ceil(docs.size() / batch_size)
    auto num_batches = (docs.size() + batch_size - 1) / batch_size;
    for (uint64_t i = 0; i < num_batches; ++i)
    {
        LOG(progress) << "\rTraining batch " << i + 1 << "/" << num_batches
                      << ENDLG;
        auto end = std::min<uint64_t>((i + 1) * batch_size, docs.size());
        std::vector<doc_id> batch{docs.begin() + (i * batch_size),
                                  docs.begin() + end};
        idx.clear_cache();
        cls.train(batch);
    }
    LOG(progress) << '\n' << ENDLG;
}

}
}
#endif
