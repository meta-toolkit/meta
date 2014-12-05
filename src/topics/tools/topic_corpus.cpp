/**
 * @file topic_corpus.cpp
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "cpptoml.h"
#include "util/filesystem.h"

using namespace meta;

int print_usage(const std::string& name)
{
    std::cerr
        << "Usage: " << name
        << " config_file model.theta\n"
           "\tCreates a line_corpus dataset based on the topics from an LDA run"
        << std::endl;
    return 1;
}

std::vector<size_t> get_topic_ids(std::ifstream& thetas)
{
    std::vector<size_t> topic_ids;
    std::string line;
    while (thetas)
    {
        std::getline(thetas, line);
        if (line.empty())
            continue;
        std::istringstream stream{line};
        size_t did;
        stream >> did;

        std::string to_split;
        size_t best_topic = 0;
        double best_prob = 0;
        while (stream)
        {
            stream >> to_split;
            if (to_split.length() == 0)
                continue;
            size_t idx = to_split.find_first_of(':');
            size_t topic{std::stoul(to_split.substr(0, idx))};
            double prob = std::stod(to_split.substr(idx + 1));
            if (prob > best_prob)
            {
                best_topic = topic;
                best_prob = prob;
            }
        }
        topic_ids.push_back(best_topic);
    }
    std::cout << "Found " << topic_ids.size() << " documents." << std::endl;
    return topic_ids;
}

void create_topic_corpus(const std::string& prefix, const std::string& dataset,
                         std::ifstream& thetas)
{
    auto topic_ids = get_topic_ids(thetas);
    auto new_file = prefix + "/" + dataset + "/" + dataset
                    + "-topics.dat.labels";
    std::ofstream out_labels{new_file};
    for (auto& topic : topic_ids)
        out_labels << "t" << topic << std::endl;
    std::cout << "Saved new labels file: " << new_file << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        return print_usage(argv[0]);

    auto config = cpptoml::parse_file(argv[1]);

    auto ctype = *config.get_as<std::string>("corpus-type");
    if (ctype != "line-corpus")
    {
        std::cerr << "Currently only line_corpus format is supported!"
                  << std::endl;
        return 1;
    }

    auto prefix = *config.get_as<std::string>("prefix");
    auto dataset = *config.get_as<std::string>("dataset");
    std::ifstream thetas{argv[2]};
    create_topic_corpus(prefix, dataset, thetas);
}
