/**
 * @file liblinear_svm.cpp
 * @author Sean Massung
 */

#include <fstream>
#include "util/invertible_map.h"
#include "classify/classifier/liblinear_svm.h"

namespace meta {
namespace classify {

liblinear_svm::liblinear_svm(std::unique_ptr<index::forward_index> & idx,
                             const std::string & liblinear_path):
    classifier{idx},
    _liblinear_path{liblinear_path}
{ /* nothing */ }

class_label liblinear_svm::classify(doc_id d_id)
{
    // create input for liblinear
    std::ofstream out("liblinear-input");
    out << _idx->liblinear_data(d_id);
    out.close();

    // run liblinear
    std::string command = _liblinear_path + "/predict liblinear-input liblinear-train.model liblinear-predicted";
    command += " 2>&1> /dev/null";
    system(command.c_str());

    // extract answer
    std::ifstream in("liblinear-predicted");
    std::string str_val;
    std::getline(in, str_val);
    in.close();

    return _idx->class_label_from_id(std::stoul(str_val));
}

confusion_matrix liblinear_svm::test(const std::vector<doc_id> & docs)
{
    // create input for liblinear
    std::ofstream out("liblinear-input");
    for(auto & d_id: docs)
        out << _idx->liblinear_data(d_id);
    out.close();

    // run liblinear
    std::string command = _liblinear_path + "/predict liblinear-input liblinear-train.model liblinear-predicted";
    command += " 2>&1> /dev/null";
    system(command.c_str());

    // extract answer
    confusion_matrix matrix;
    std::ifstream in("liblinear-predicted");
    std::string str_val;
    for(auto & d_id: docs)
    {
        // we can assume that the number of lines in the file is equal to the
        // number of testing documents
        std::getline(in, str_val);
        int value = std::stoul(str_val) - 1; // get correct start for liblinear
        class_label predicted = _idx->class_label_from_id(value);
        class_label actual = _idx->label(d_id);
        matrix.add(predicted, actual);
    }
    in.close();

    return matrix;
}

void liblinear_svm::train(const std::vector<doc_id> & docs)
{
    std::ofstream out("liblinear-train");
    for(auto & d_id: docs)
        out << _idx->liblinear_data(d_id);
    out.close();

    std::string command = _liblinear_path + "/train liblinear-train";
    command += " 2>&1> /dev/null";
    system(command.c_str());
}

void liblinear_svm::reset()
{
    // nothing
}

}
}
