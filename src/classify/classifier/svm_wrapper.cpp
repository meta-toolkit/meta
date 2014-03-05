/**
 * @file svm_wrapper.cpp
 * @author Sean Massung
 */

#include <fstream>
#include "util/invertible_map.h"
#include "classify/classifier/svm_wrapper.h"

namespace meta {
namespace classify {

decltype(svm_wrapper::_options) svm_wrapper::_options = {
    {svm_wrapper::kernel::None,      ""},
    {svm_wrapper::kernel::Quadratic, " -t 1 -d 2 "},
    {svm_wrapper::kernel::Cubic,     " -t 1 -d 3 "},
    {svm_wrapper::kernel::Quartic,   " -t 1 -d 4 "},
    {svm_wrapper::kernel::RBF,       " -t 2 "},
    {svm_wrapper::kernel::Sigmoid,   " -t 3 "}
};

svm_wrapper::svm_wrapper(std::shared_ptr<index::forward_index> idx,
                         const std::string & svm_path,
                         kernel kernel_opt /* = None */):
    classifier{std::move(idx)},
    _svm_path{svm_path},
    _kernel{kernel_opt}
{
    if(kernel_opt == kernel::None)
        _executable = "liblinear/";
    else
        _executable = "libsvm/svm-";
}

class_label svm_wrapper::classify(doc_id d_id)
{
    // create input for liblinear
    std::ofstream out("svm-input");
    out << _idx->liblinear_data(d_id);
    out.close();

    // run liblinear/libsvm
    std::string command = _svm_path + _executable
        + "predict svm-input svm-train.model svm-predicted";
    command += " > /dev/null 2>&1";
    system(command.c_str());

    // extract answer
    std::ifstream in("svm-predicted");
    std::string str_val;
    std::getline(in, str_val);
    in.close();

    label_id label{static_cast<uint32_t>(std::stoul(str_val))};
    return _idx->class_label_from_id(label);
}

confusion_matrix svm_wrapper::test(const std::vector<doc_id> & docs)
{
    // create input for liblinear/libsvm
    std::ofstream out("svm-input");
    for(auto & d_id: docs)
        out << _idx->liblinear_data(d_id) << "\n";
    out.close();

    // run liblinear/libsvm
    std::string command = _svm_path + _executable
        + "predict svm-input svm-train.model svm-predicted";
    command += " > /dev/null 2>&1";
    system(command.c_str());

    // extract answer
    confusion_matrix matrix;
    std::ifstream in("svm-predicted");
    std::string str_val;
    for(auto & d_id: docs)
    {
        // we can assume that the number of lines in the file is equal to the
        // number of testing documents
        std::getline(in, str_val);
        uint32_t value = std::stoul(str_val);
        class_label predicted = _idx->class_label_from_id(label_id{value});
        class_label actual = _idx->label(d_id);
        matrix.add(predicted, actual);
    }
    in.close();

    return matrix;
}

void svm_wrapper::train(const std::vector<doc_id> & docs)
{
    std::ofstream out("svm-train");
    for(auto & d_id: docs)
        out << _idx->liblinear_data(d_id) << "\n";
    out.close();

    std::string command = _svm_path + _executable + "train "
        + _options.at(_kernel) + " svm-train";
    command += " > /dev/null 2>&1";
    system(command.c_str());
}

void svm_wrapper::reset()
{
    // nothing
}

}
}
