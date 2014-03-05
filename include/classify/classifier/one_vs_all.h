/**
 * @file sgd.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_ONE_VS_ALL_H_
#define _META_CLASSIFY_ONE_VS_ALL_H_

#include "classify/classifier/classifier.h"
#include "meta.h"

namespace meta {
namespace classify {

template <class Classifier>
class one_vs_all : public classifier {
    public:
        template <class... Args>
        one_vs_all(index::forward_index & idx, Args &&... args);

        void train(const std::vector<doc_id> & docs) override;

        class_label classify( doc_id d_id ) override;

        void reset() override;

    private:
        std::unordered_map<class_label, Classifier> classifiers_;
};

}
}
#include "classify/classifier/one_vs_all.tcc"
#endif
