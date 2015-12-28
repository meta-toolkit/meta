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
#include "meta/io/filesystem.h"

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
    while (std::getline(thetas, line))
    {
        if (line.empty())
            continue;
        std::istringstream stream{line};
        size_t did;
        stream >> did;

        std::string to_split;
        size_t best_topic = 0;
        double best_prob = 0;
        while (stream >> to_split)
        {
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
    size_t num_topics = 0;
    for (auto& topic : topic_ids)
    {
        if (topic > num_topics)
            num_topics = topic;
        out_labels << "t" << topic << std::endl;
    }
    ++num_topics; // total is one more than highest id
    std::cout << "Saved new labels file: " << new_file << std::endl;

    // for each topic, what is the distribution of original class labels?

    auto labels_file = prefix + "/" + dataset + "/" + dataset + ".dat.labels";
    std::ifstream orig_labels_in{labels_file};
    std::vector<std::string> orig_labels;
    std::string buf;
    while (orig_labels_in >> buf)
        orig_labels.push_back(buf);

    std::cout << orig_labels.size() << std::endl;

    std::unordered_map<std::string, std::vector<double>> counts;
    uint64_t idx = 0;
    for (auto& topic : topic_ids)
    {
        auto label = orig_labels[idx];
        if (counts[label].empty())
            counts[label].resize(num_topics, 0.0);
        ++counts[label][topic];
        ++idx;
    }

    std::ofstream out_dist{dataset + ".topic-dist"};
    for (auto& label : counts)
    {
        out_dist << label.first;
        double total = 0.0;
        for (const auto& count : label.second)
            total += count;
        for (const auto& count : label.second)
            out_dist << "\t" << count / total;
        out_dist << std::endl;
    }

    std::cout << "Saved topic dist file: " << (dataset + ".topic-dist")
              << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        return print_usage(argv[0]);

    auto config = cpptoml::parse_file(argv[1]);

    auto ctype = *config->get_as<std::string>("corpus-type");
    if (ctype != "line-corpus")
    {
        std::cerr << "Currently only line_corpus format is supported!"
                  << std::endl;
        return 1;
    }

    auto prefix = *config->get_as<std::string>("prefix");
    auto dataset = *config->get_as<std::string>("dataset");
    std::ifstream thetas{argv[2]};
    create_topic_corpus(prefix, dataset, thetas);
}
