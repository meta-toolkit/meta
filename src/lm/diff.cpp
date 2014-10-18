/**
 * @file diff.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <queue>
#include "lm/diff.h"
#include "cpptoml.h"
#include "porter2_stemmer.h"

namespace meta
{
namespace lm
{
diff::diff(const std::string& config_file, uint64_t max_depth)
    : lm_{config_file}, max_depth_{max_depth}
{
    set_stems(config_file);
    set_function_words(config_file);
}

std::vector<std::pair<sentence, double>> diff::candidates(const sentence& sent)
{
    using pair_t = std::pair<sentence, double>;
    auto comp = [](const pair_t& a, const pair_t& b)
    {
        return a.second < b.second;
    };
    std::priority_queue<pair_t, std::vector<pair_t>, decltype(comp)> candidates{
        comp};
    candidates.emplace(sent, lm_.perplexity_per_word(sent));

    seen_.clear();
    step(sent, candidates, 0);

    std::vector<pair_t> sorted;
    while (!candidates.empty())
    {
        sorted.push_back(candidates.top());
        candidates.pop();
    }
    std::reverse(sorted.begin(), sorted.end());
    return sorted;
}

template <class PQ>
void diff::step(const sentence& sent, PQ& candidates, size_t depth)
{
    if (depth == max_depth_)
        return;

    for (size_t i = 0; i < sent.size(); ++i)
    {
        // remove

        sentence rem_cpy{sent};
        rem_cpy.remove(i);
        if (seen_.find(rem_cpy.to_string()) == seen_.end())
        {
            seen_.insert(rem_cpy.to_string());
            candidates.emplace(rem_cpy, lm_.perplexity_per_word(rem_cpy));
            step(rem_cpy, candidates, depth + 1);
        }

        // insert

        for (auto& fw : fwords_)
        {
            sentence ins_cpy{sent};
            ins_cpy.insert(i, fw);
            if (seen_.find(ins_cpy.to_string()) == seen_.end())
            {
                seen_.insert(ins_cpy.to_string());
                candidates.emplace(ins_cpy, lm_.perplexity_per_word(ins_cpy));
                step(ins_cpy, candidates, depth + 1);
            }
        }

        // substitute

        std::string stemmed = sent[i];
        Porter2Stemmer::stem(stemmed);
        auto it = stems_.find(stemmed);
        if (it != stems_.end() && it->second.size() != 1)
        {
            for (auto& stem : it->second)
            {
                sentence subbed{sent};
                subbed.substitute(i, stem);
                if (seen_.find(subbed.to_string()) == seen_.end())
                {
                    seen_.insert(subbed.to_string());
                    candidates.emplace(subbed, lm_.perplexity_per_word(subbed));
                    step(subbed, candidates, depth + 1);
                }
            }
        }
    }
}

void diff::set_function_words(const std::string& config_file)
{
    auto config = cpptoml::parse_file(config_file);
    std::ifstream in{*config.get_as<std::string>("function-words")};
    std::string word;
    while (in >> word)
        fwords_.push_back(word);
}

void diff::set_stems(const std::string& config_file)
{
    std::unordered_set<std::string> vocab;
    auto config = cpptoml::parse_file(config_file);
    auto prefix = *config.get_as<std::string>("prefix");
    auto dataset = *config.get_as<std::string>("dataset");
    std::ifstream in{prefix + "/" + dataset + "/" + dataset + ".dat"};
    std::string token;
    while (in >> token)
    {
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        vocab.insert(token);
    }

    for (auto& t : vocab)
    {
        std::string stemmed{t};
        Porter2Stemmer::stem(stemmed);
        stems_[stemmed].push_back(t);
    }
}
}
}
