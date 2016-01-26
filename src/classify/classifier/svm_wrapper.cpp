/**
 * @file svm_wrapper.cpp
 * @author Sean Massung
 */

#include <fstream>
#include "meta/classify/classifier/svm_wrapper.h"
#include "meta/utf/utf.h"

namespace meta
{
namespace classify
{

const util::string_view svm_wrapper::id = "libsvm";

decltype(svm_wrapper::options_) svm_wrapper::options_
    = {{svm_wrapper::kernel::None, ""},
       {svm_wrapper::kernel::Quadratic, " -t 1 -d 2 "},
       {svm_wrapper::kernel::Cubic, " -t 1 -d 3 "},
       {svm_wrapper::kernel::Quartic, " -t 1 -d 4 "},
       {svm_wrapper::kernel::RBF, " -t 2 "},
       {svm_wrapper::kernel::Sigmoid, " -t 3 "}};

svm_wrapper::svm_wrapper(dataset_view_type docs, const std::string& svm_path,
                         kernel kernel_opt /* = None */)
    : svm_path_{svm_path}, kernel_{kernel_opt}
{

    labels_.resize(docs.total_labels());
    for (auto it = docs.labels_begin(), end = docs.labels_end(); it != end;
         ++it)
    {
        labels_.at(it->second) = it->first;
    }

    if (kernel_opt == kernel::None)
        executable_ = "liblinear/build/";
    else
        executable_ = "libsvm/build/svm-";

    {
        std::ofstream out{"svm-train"};
        for (const auto& instance : docs)
        {
            docs.print_liblinear(out, instance);
            out << "\n";
        }
    }

#ifndef _WIN32
    std::string command = svm_path_ + executable_ + "train "
                          + options_.at(kernel_) + " svm-train";
    command += " > /dev/null 2>&1";
#else
    // see comment in classify()
    auto command = "\"\"" + svm_path_ + executable_ + "train.exe\" "
                   + options_.at(kernel_) + " svm-train";
    command += " > NUL 2>&1\"";
#endif
    system(command.c_str());
}

svm_wrapper::svm_wrapper(std::istream& in)
    : svm_path_{io::packed::read<std::string>(in)}
{
    io::packed::read(in, kernel_);
    io::packed::read(in, executable_);

    auto size = io::packed::read<std::size_t>(in);
    labels_.resize(size);
    for (std::size_t i = 0; i < size; ++i)
        io::packed::read(in, labels_[i]);

    std::ofstream out{"svm-train.model"};
    auto model_lines = io::packed::read<std::size_t>(in);
    std::string line;
    for (std::size_t i = 0; i < model_lines; ++i)
    {
        std::getline(in, line);
        out << line << "\n";
    }
}

void svm_wrapper::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, svm_path_);
    io::packed::write(out, kernel_);
    io::packed::write(out, executable_);

    io::packed::write(out, labels_.size());
    for (const auto& lbl : labels_)
        io::packed::write(out, lbl);

    auto num_lines = filesystem::num_lines("svm-train.model");
    io::packed::write(out, num_lines);
    std::ifstream in{"svm-train.model"};
    std::string line;
    for (std::size_t i = 0; i < num_lines; ++i)
    {
        std::getline(in, line);
        out << line << "\n";
    }
}

class_label svm_wrapper::classify(const feature_vector& doc) const
{
    // create input for liblinear
    {
        std::ofstream out{"svm-input"};
        out << "1 "; // dummy label
        learn::print_liblinear(out, doc);
        out << "\n";
    }

// run liblinear/libsvm
#ifndef _WIN32
    std::string command = svm_path_ + executable_
                          + "predict svm-input svm-train.model svm-predicted";
    command += " > /dev/null 2>&1";
#else
    // first set of quotes is around the exe name to make things work without
    // having to use forward slashes in the path name.
    //
    // second set of quotes is around the entire command, since Windows does
    // strange things in making the command to actually be sent to CMD.exe
    auto command = "\"\"" + svm_path_ + executable_
                   + "predict.exe\" svm-input svm-train.model svm-predicted";
    command += " > NUL 2>&1\"";
#endif
    system(command.c_str());

    // extract answer
    std::string str_val;
    {
        std::ifstream in{"svm-predicted"};
        std::getline(in, str_val);
    }

    auto lbl = std::stoul(str_val);
    assert(lbl > 0);
    return labels_.at(lbl - 1);
}

confusion_matrix svm_wrapper::test(multiclass_dataset_view docs) const
{
    // create input for liblinear/libsvm
    {
        std::ofstream out{"svm-input"};
        for (const auto& instance : docs)
        {
            docs.print_liblinear(out, instance);
            out << "\n";
        }
    }

// run liblinear/libsvm
#ifndef _WIN32
    std::string command = svm_path_ + executable_
                          + "predict svm-input svm-train.model svm-predicted";
    command += " > /dev/null 2>&1";
#else
    // see comment in classify()
    auto command = "\"\"" + svm_path_ + executable_
                   + "predict.exe\" svm-input svm-train.model svm-predicted";
    command += " > NUL 2>&1\"";
#endif
    system(command.c_str());

    // extract answer
    confusion_matrix matrix;
    {
        std::ifstream in{"svm-predicted"};
        std::string str_val;
        for (const auto& instance : docs)
        {
            // we can assume that the number of lines in the file is equal
            // to the number of testing documents
            std::getline(in, str_val);
            auto value = std::stoul(str_val);
            assert(value > 0);
            predicted_label predicted{labels_.at(value - 1)};
            class_label actual = docs.label(instance);
            matrix.add(predicted, actual);
        }
    }

    return matrix;
}

template <>
std::unique_ptr<classifier>
    make_classifier<svm_wrapper>(const cpptoml::table& config,
                                 multiclass_dataset_view training)
{
    auto path = config.get_as<std::string>("path");
    if (!path)
        throw classifier_factory::exception{
            "path to libsvm modules must be present in config for svm wrapper"};

    if (auto kernel = config.get_as<std::string>("kernel"))
    {
        auto k_type = utf::tolower(*kernel);
        if (k_type == "none")
            return make_unique<svm_wrapper>(std::move(training), *path);

        if (k_type == "quadratic")
            return make_unique<svm_wrapper>(std::move(training), *path,
                                            svm_wrapper::kernel::Quadratic);

        if (k_type == "cubic")
            return make_unique<svm_wrapper>(std::move(training), *path,
                                            svm_wrapper::kernel::Cubic);

        if (k_type == "quartic")
            return make_unique<svm_wrapper>(std::move(training), *path,
                                            svm_wrapper::kernel::Quartic);

        if (k_type == "rbf")
            return make_unique<svm_wrapper>(std::move(training), *path,
                                            svm_wrapper::kernel::RBF);

        if (k_type == "sigmoid")
            return make_unique<svm_wrapper>(std::move(training), *path,
                                            svm_wrapper::kernel::Sigmoid);
    }

    return make_unique<svm_wrapper>(std::move(training), *path);
}
}
}
