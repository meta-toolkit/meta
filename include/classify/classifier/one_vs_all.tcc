#include "classify/classifier/one_vs_all.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

template <class Classifier>
template <class... Args>
one_vs_all<Classifier>::one_vs_all(index::forward_index & idx,
                                   Args &&... args) : classifier{idx} {
    for (const auto & d_id : idx.docs()) {
        if (classifiers_.find(idx.label(d_id)) != classifiers_.end())
            continue;
        classifiers_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(idx.label(d_id)),
                std::forward_as_tuple(idx,
                                      idx.label(d_id),
                                      std::forward<Args>(args)...));
    }
}

template <class Classifier>
void one_vs_all<Classifier>::train(const std::vector<doc_id> & docs) {
    parallel::parallel_for(classifiers_.begin(), classifiers_.end(),
        [&](decltype(*classifiers_.begin()) p) {
            p.second.train(docs);
        });
}

template <class Classifier>
class_label one_vs_all<Classifier>::classify(doc_id d_id) {
    class_label best_label;
    double best_prediction = std::numeric_limits<double>::lowest();
    for (auto & pair : classifiers_) {
        double prediction = pair.second.predict(d_id);
        if (prediction > best_prediction) {
            best_prediction = prediction;
            best_label = pair.first;
        }
    }
    return best_label;
}

template <class Classifier>
void one_vs_all<Classifier>::reset() {
    for (auto & pair : classifiers_)
        pair.second.reset();
}

}
}
