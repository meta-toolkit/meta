/**
 * @file corpus_gen.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <string>
#include <exception>
#include <algorithm>
#include "cpptoml.h"
#include "meta/io/filesystem.h"
#include "meta/util/printing.h"
#include "meta/meta.h"

using namespace meta;

std::string get_content(const std::string& file, const std::string& prefix)
{
    std::ifstream input{prefix + file, std::ios::binary};
    std::ostringstream oss;
    oss << input.rdbuf();
    std::string content{oss.str()};
    std::replace_if(content.begin(), content.end(), [](char ch) {
        return ch == '\n' || ch == '\t'; }, ' ');
    return content;
}

void create_line_corpus(const std::string& filename,
                        const std::string& new_filename,
                        const std::string& prefix)
{
    std::ifstream input_paths{filename};
    if (!input_paths.good())
        std::cout << "Failed to open " << filename << std::endl;
    std::ofstream content{new_filename};
    std::ofstream labels{new_filename + ".labels"};
    std::ofstream names{new_filename + ".names"};

    uint64_t num_lines = filesystem::num_lines(filename);
    uint64_t cur_line = 0;
    std::cout << "Found " << num_lines << " files" << std::endl;

    std::string path;
    std::string label;
    while (input_paths >> label >> path)
    {
        content << get_content(path, prefix) << "\n";
        labels << label << "\n";
        names << path << "\n";
        std::cout << ++cur_line << "/" << num_lines << " " << path
                  << "\t\t\t\t\r";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    auto config = cpptoml::parse_file(argv[1]);
    auto prefix = config->get_as<std::string>("prefix");
    if (!prefix)
        throw std::runtime_error{"prefix missing from configuration file"};

    auto dataset = config->get_as<std::string>("dataset");
    if (!dataset)
        throw std::runtime_error{"dataset missing from configuration file"};

    auto file_list = config->get_as<std::string>("list");
    if (!file_list)
        throw std::runtime_error{"list missing from configuration file"};

    std::string file =
        *prefix + "/" + *dataset + "/" + *file_list + "-full-corpus.txt";
    std::string new_file = *prefix + "/" + *dataset + "/" + *dataset + ".dat";

    create_line_corpus(file, new_file, *prefix + "/" + *dataset + "/");

    return 0;
}
