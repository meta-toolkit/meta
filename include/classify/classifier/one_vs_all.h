/**
 * @file one_vs_all.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_ONE_VS_ALL_H_
#define META_CLASSIFY_ONE_VS_ALL_H_

#include "classify/classifier/binary_classifier.h"
#include "classify/classifier_factory.h"
#include "meta.h"

namespace meta
{
namespace classify
{

class one_vs_all : public classifier
{
  public:
    template <class Function>
    one_vs_all(std::shared_ptr<index::forward_index> idx, Function&& create)
        : classifier{std::move(idx)}
    {
        for (const auto& d_id : idx_->docs())
        {
            if (classifiers_.find(idx_->label(d_id)) != classifiers_.end())
                continue;
            classifiers_.emplace(idx_->label(d_id), create(idx_->label(d_id)));
        }
    }

    void train(const std::vector<doc_id>& docs) override;

    class_label classify(doc_id d_id) override;

    void reset() override;

    const static std::string id;

  private:
    std::unordered_map<class_label, std::unique_ptr<binary_classifier>>
        classifiers_;
};

/**
 * Specialization of the factory method used to create one_vs_all
 * classifiers.
 */
template <>
std::unique_ptr<classifier>
    make_classifier<one_vs_all>(const cpptoml::toml_group&,
                                std::shared_ptr<index::forward_index> idx);
}
}
#endif
