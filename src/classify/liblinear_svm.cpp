/**
 * @file liblinear_svm.cpp
 * @author Sean Massung
 */

#include <fstream>
#include "util/invertible_map.h"
#include "classify/liblinear_svm.h"

namespace meta {
namespace classify {

using std::string;
using std::vector;
using index::document;

liblinear_svm::liblinear_svm(const string & liblinear_path):
    _liblinear_path(liblinear_path),
    _mapping(util::invertible_map<class_label, int>())
{ /* nothing */ }

class_label liblinear_svm::classify(const document & doc)
{
    // create input for liblinear
    std::ofstream out("liblinear-input");
    out << doc.get_liblinear_data(_mapping);
    out.close();

    // run liblinear
    string command = _liblinear_path + "/predict liblinear-input liblinear-train.model liblinear-predicted";
    command += " 2>&1> /dev/null";
    system(command.c_str());

    // extract answer
    std::ifstream in("liblinear-predicted");
    string str_val;
    std::getline(in, str_val);
    in.close();
    int value = std::stoul(str_val);

    return _mapping.get_key(value);
}

confusion_matrix liblinear_svm::test(const vector<document> & docs)
{
    // create input for liblinear
    std::ofstream out("liblinear-input");
    for(auto & d: docs)
        out << d.get_liblinear_data(_mapping);
    out.close();

    // run liblinear
    string command = _liblinear_path + "/predict liblinear-input liblinear-train.model liblinear-predicted";
    command += " 2>&1> /dev/null";
    system(command.c_str());

    // extract answer
    confusion_matrix matrix;
    std::ifstream in("liblinear-predicted");
    string str_val;
    for(auto & d: docs)
    {
        // we can assume that the number of lines in the file is equal to the
        // number of testing documents
        std::getline(in, str_val);
        int value = std::stoul(str_val);
        matrix.add(_mapping.get_key(value), d.label());
    }
    in.close();

    return matrix;
}

void liblinear_svm::train(const vector<document> & docs)
{
    std::ofstream out("liblinear-train");
    for(auto & d: docs)
        out << d.get_liblinear_data(_mapping);
    out.close();

    string command = _liblinear_path + "/train liblinear-train";
    command += " 2>&1> /dev/null";
    system(command.c_str());
}

void liblinear_svm::reset()
{
    _mapping.clear();
}

}
}
