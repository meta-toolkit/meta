/**
 * @file svm_wrapper.cpp
 * @author Sean Massung
 */

#include <fstream>
#include "classify/classifier/svm_wrapper.h"
#include "utf/utf.h"

namespace meta
{
namespace classify
{

const std::string svm_wrapper::id = "libsvm";

decltype(svm_wrapper::options_) svm_wrapper::options_
    = {{svm_wrapper::kernel::None, ""},
       {svm_wrapper::kernel::Quadratic, " -t 1 -d 2 "},
       {svm_wrapper::kernel::Cubic, " -t 1 -d 3 "},
       {svm_wrapper::kernel::Quartic, " -t 1 -d 4 "},
       {svm_wrapper::kernel::RBF, " -t 2 "},
       {svm_wrapper::kernel::Sigmoid, " -t 3 "}};

svm_wrapper::svm_wrapper(std::shared_ptr<index::forward_index> idx,
                         const std::string& svm_path,
                         kernel kernel_opt /* = None */)
    : classifier{std::move(idx)}, svm_path_{svm_path}, kernel_{kernel_opt}
{
    if (kernel_opt == kernel::None)
        executable_ = "liblinear/";
    else
        executable_ = "libsvm/svm-";
}

class_label svm_wrapper::classify(doc_id d_id)
{
    // create input for liblinear
    std::ofstream out("svm-input");
    out << idx_->liblinear_data(d_id);
    out.close();

    // run liblinear/libsvm
    std::string command = svm_path_ + executable_
                          + "predict svm-input svm-train.model svm-predicted";
    command += " > /dev/null 2>&1";
    system(command.c_str());

    // extract answer
    std::ifstream in("svm-predicted");
    std::string str_val;
    std::getline(in, str_val);
    in.close();

    label_id label{static_cast<uint32_t>(std::stoul(str_val))};
    return idx_->class_label_from_id(label);
}

confusion_matrix svm_wrapper::test(const std::vector<doc_id>& docs)
{
    // create input for liblinear/libsvm
    std::ofstream out("svm-input");
    for (auto& d_id : docs)
        out << idx_->liblinear_data(d_id) << "\n";
    out.close();

    // run liblinear/libsvm
    std::string command = svm_path_ + executable_
                          + "predict svm-input svm-train.model svm-predicted";
    command += " > /dev/null 2>&1";
    system(command.c_str());

    // extract answer
    confusion_matrix matrix;
    std::ifstream in("svm-predicted");
    std::string str_val;
    for (auto& d_id : docs)
    {
        // we can assume that the number of lines in the file is equal to the
        // number of testing documents
        std::getline(in, str_val);
        uint32_t value = std::stoul(str_val);
        class_label predicted = idx_->class_label_from_id(label_id{value});
        class_label actual = idx_->label(d_id);
        matrix.add(predicted, actual);
    }
    in.close();

    return matrix;
}

void svm_wrapper::train(const std::vector<doc_id>& docs)
{
    std::ofstream out("svm-train");
    for (auto& d_id : docs)
        out << idx_->liblinear_data(d_id) << "\n";
    out.close();

    std::string command = svm_path_ + executable_ + "train "
                          + options_.at(kernel_) + " svm-train";
    command += " > /dev/null 2>&1";
    system(command.c_str());
}

void svm_wrapper::reset()
{
    // nothing
}

template <>
std::unique_ptr<classifier>
    make_classifier<svm_wrapper>(const cpptoml::table& config,
                                 std::shared_ptr<index::forward_index> idx)
{
    auto path = config.get_as<std::string>("path");
    if (!path)
        throw classifier_factory::exception{
            "path to libsvm modules must be present in config for svm wrapper"};

    if (auto kernel = config.get_as<std::string>("kernel"))
    {
        auto k_type = utf::tolower(*kernel);
        if (k_type == "none")
            return make_unique<svm_wrapper>(std::move(idx), *path);

        if (k_type == "quadratic")
            return make_unique<svm_wrapper>(std::move(idx), *path,
                                            svm_wrapper::kernel::Quadratic);

        if (k_type == "cubic")
            return make_unique<svm_wrapper>(std::move(idx), *path,
                                            svm_wrapper::kernel::Cubic);

        if (k_type == "quartic")
            return make_unique<svm_wrapper>(std::move(idx), *path,
                                            svm_wrapper::kernel::Quartic);

        if (k_type == "rbf")
            return make_unique<svm_wrapper>(std::move(idx), *path,
                                            svm_wrapper::kernel::RBF);

        if (k_type == "sigmoid")
            return make_unique<svm_wrapper>(std::move(idx), *path,
                                            svm_wrapper::kernel::Sigmoid);
    }

    return make_unique<svm_wrapper>(std::move(idx), *path);
}
}
}
