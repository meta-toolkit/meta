/**
 * @file classifier.cpp
 * @author Sean Massung
 */

#include <random>
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

confusion_matrix classifier::cross_validate(
        const std::vector<doc_id>& input_docs,
        size_t k, int seed)
{
    // docs might be ordered by class, so make sure things are shuffled
    std::vector<doc_id> docs{input_docs};
    std::mt19937 gen(seed);
    std::shuffle(docs.begin(), docs.end(), gen);

    confusion_matrix matrix;
    size_t step_size = docs.size() / k;
    for (size_t i = 0; i < k; ++i)
    {
        LOG(info) << "Cross-validating fold " << (i + 1) << "/" << k
                      << ENDLG;
        reset(); // clear any learning data already calculated
        train(std::vector<doc_id>{docs.begin() + step_size, docs.end()});
        auto m = test(std::vector<doc_id>{docs.begin(), docs.begin() + step_size});
        LOG(info) << " -> Accuracy: " << m.accuracy() << ENDLG;
        std::cout << m.accuracy() << std::endl;
        matrix += m;
        std::rotate(docs.begin(), docs.begin() + step_size, docs.end());
    }
    LOG(info) << '\n' << ENDLG;

    return matrix;
}
}
}
