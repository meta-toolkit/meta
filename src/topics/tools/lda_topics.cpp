/**
 * @file lda_topics.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "meta/caching/no_evict_cache.h"
#include "meta/index/forward_index.h"
#include "meta/util/fixed_heap.h"

using namespace meta;

int print_usage(const std::string& name)
{
    std::cout
        << "Usage: " << name
        << " config_file model.phi num_words \n"
           "\tPrints the top num_words words in each topic in the given model"
        << std::endl;
    return 1;
}

int print_topics(const std::string& config_file, const std::string& filename,
                 size_t num_words)
{
    auto config = cpptoml::parse_file(config_file);
    auto idx = index::make_index<index::forward_index, caching::no_evict_cache>(
        *config);

    std::ifstream file{filename};
    while (file)
    {
        std::string line;
        std::getline(file, line);
        if (line.length() == 0)
            continue;
        std::stringstream stream(line);
        size_t topic;
        stream >> topic;
        std::cout << "Topic " << topic << ":" << std::endl;
        std::cout << "-----------------------" << std::endl;

        auto comp = [](const std::pair<term_id, double>& first,
                       const std::pair<term_id, double>& second)
        {
            return first.second > second.second;
        };
        util::fixed_heap<std::pair<term_id, double>, decltype(comp)> pairs{
            num_words, comp};

        while (stream)
        {
            std::string to_split;
            stream >> to_split;
            if (to_split.length() == 0)
                continue;
            size_t idx = to_split.find_first_of(':');
            term_id term{std::stoul(to_split.substr(0, idx))};
            double prob = std::stod(to_split.substr(idx + 1));
            pairs.emplace(term, prob);
        }
        for (const auto& p : pairs.extract_top())
            std::cout << idx->term_text(p.first) << " (" << p.first
                      << "): " << p.second << std::endl;
        std::cout << std::endl;
    }
    return 0;
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() < 4)
        return print_usage(args[0]);
    return print_topics(args[1], args[2], std::stoul(argv[3]));
}
