/**
 * @file one_vs_all.cpp
 * @author Chase Geigle
 */

#include "classify/binary_classifier_factory.h"
#include "classify/classifier/one_vs_all.h"
#include "parallel/parallel_for.h"

namespace meta
{
namespace classify
{

const std::string one_vs_all::id = "one-vs-all";

void one_vs_all::train(const std::vector<doc_id>& docs)
{
    parallel::parallel_for(classifiers_.begin(), classifiers_.end(),
                           [&](decltype(*classifiers_.begin()) p)
    { p.second->train(docs); });
}

class_label one_vs_all::classify(doc_id d_id)
{
    class_label best_label;
    double best_prediction = std::numeric_limits<double>::lowest();
    for (auto& pair : classifiers_)
    {
        double prediction = pair.second->predict(d_id);
        if (prediction > best_prediction)
        {
            best_prediction = prediction;
            best_label = pair.first;
        }
    }
    return best_label;
}

void one_vs_all::reset()
{
    for (auto& pair : classifiers_)
        pair.second->reset();
}

template <>
std::unique_ptr<classifier>
    make_classifier<one_vs_all>(const cpptoml::table& config,
                                std::shared_ptr<index::forward_index> idx)
{
    auto base = config.get_table("base");
    if (!base)
        throw classifier_factory::exception{
            "one-vs-all missing base-classifier parameter in config file"};
    return make_unique<one_vs_all>(idx, [&](class_label positive_label)
    {
        return make_binary_classifier(*base, idx, positive_label,
                                      class_label{"negative"});
    });
}
}
}
