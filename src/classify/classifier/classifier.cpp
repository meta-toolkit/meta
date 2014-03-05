/**
 * @file classifier.tcc
 * @author Sean Massung
 */

#include <random>
#include "logging/logger.h"
#include "classify/classifier/classifier.h"

namespace meta {
namespace classify {

classifier::classifier(index::forward_index & idx):
    _idx(idx)
{ /* nothing */ }

confusion_matrix classifier::test(const std::vector<doc_id> & docs)
{
    confusion_matrix matrix;
    for(auto & d_id: docs)
        matrix.add(classify(d_id), _idx.label(d_id));

    return matrix;
}

confusion_matrix classifier::cross_validate(
        const std::vector<doc_id> & input_docs,
        size_t k, int seed)
{
    // docs might be ordered by class, so make sure things are shuffled
    std::vector<doc_id> docs{input_docs};
    std::mt19937 gen(seed);
    std::shuffle(docs.begin(), docs.end(), gen);

    confusion_matrix matrix;
    size_t step_size = docs.size() / k;
    for(size_t i = 0; i < k; ++i)
    {
        LOG(progress) << "\rCross-validating fold " << (i + 1) << "/" << k << ENDLG;
        reset(); // clear any learning data already calculated
        train(std::vector<doc_id>{docs.begin() + step_size, docs.end()});
        matrix +=
            test(std::vector<doc_id>{docs.begin(), docs.begin() + step_size});
        std::rotate(docs.begin(), docs.begin() + step_size, docs.end());
    }
    LOG(progress) << '\n' << ENDLG;

    return matrix;
}

}
}
