/**
 * @file classifier.cpp
 * @author Sean Massung
 */

#include <random>
#include <numeric>
#include "logging/logger.h"
#include "classify/classifier/classifier.h"

namespace meta
{
namespace classify
{

classifier::classifier(std::shared_ptr<index::forward_index> idx)
    : idx_(std::move(idx))
{
    /* nothing */
}

confusion_matrix classifier::test(const std::vector<doc_id>& docs)
{
    confusion_matrix matrix;
    for (auto& d_id : docs)
        matrix.add(classify(d_id), idx_->label(d_id));

    return matrix;
}

confusion_matrix
    classifier::cross_validate(const std::vector<doc_id>& input_docs, size_t k,
                               bool even_split /* = false */, int seed)
{
    // docs might be ordered by class, so make sure things are shuffled
    std::vector<doc_id> docs{input_docs};
    if (even_split)
        create_even_split(docs, seed);
    std::mt19937 gen(seed);
    std::shuffle(docs.begin(), docs.end(), gen);

    confusion_matrix matrix;
    size_t step_size = docs.size() / k;
    for (size_t i = 0; i < k; ++i)
    {
        LOG(info) << "Cross-validating fold " << (i + 1) << "/" << k << ENDLG;
        reset(); // clear any learning data already calculated
        train(std::vector<doc_id>{docs.begin() + step_size, docs.end()});
        auto m
            = test(std::vector<doc_id>{docs.begin(), docs.begin() + step_size});
        matrix += m;
        std::rotate(docs.begin(), docs.begin() + step_size, docs.end());
    }
    LOG(info) << '\n' << ENDLG;

    return matrix;
}

void classifier::create_even_split(std::vector<doc_id>& docs, int seed) const
{
    LOG(info) << "Creating an even split of class labels" << ENDLG;
    std::unordered_map<label_id, std::vector<doc_id>> partitioned;
    for (auto& id : docs)
        partitioned[idx_->lbl_id(id)].push_back(id);

    uint64_t min = std::numeric_limits<uint64_t>::max();
    for (auto& bucket : partitioned)
    {
        if (bucket.second.size() < min)
            min = bucket.second.size();
    }

    std::mt19937 gen(seed);
    for (auto& bucket : partitioned)
    {
        if (bucket.second.size() > min)
        {
            std::shuffle(bucket.second.begin(), bucket.second.end(), gen);
            bucket.second.resize(min);
        }
    }

    docs.clear();
    for (auto& bucket : partitioned)
        docs.insert(docs.end(), bucket.second.begin(), bucket.second.end());

    LOG(info) << "Each of the " << partitioned.size() << " classes has " << min
              << " elements for a baseline accuracy of "
              << 1.0 / partitioned.size() << ENDLG;
}
}
}
