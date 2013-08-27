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
    // TODO
    out << d_id; // doc.get_liblinear_data(_mapping);
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
    int value = std::stoul(str_val);

    return _mapping.get_key(value);
}

confusion_matrix liblinear_svm::test(const std::vector<doc_id> & docs)
{
    // create input for liblinear
    std::ofstream out("liblinear-input");
    // TODO
    for(auto & d_id: docs)
        out << d_id; // d.get_liblinear_data(_mapping);
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
        int value = std::stoul(str_val);
        matrix.add(_mapping.get_key(value), _idx->label(d_id));
    }
    in.close();

    return matrix;
}

void liblinear_svm::train(const std::vector<doc_id> & docs)
{
    std::ofstream out("liblinear-train");
    // TODO
    for(auto & d_id: docs)
        out << d_id; // d.get_liblinear_data(_mapping);
    out.close();

    std::string command = _liblinear_path + "/train liblinear-train";
    command += " 2>&1> /dev/null";
    system(command.c_str());
}

void liblinear_svm::reset()
{
    _mapping.clear();
}

}
}
