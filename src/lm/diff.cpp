/**
 * @file diff.cpp
 * @author Sean Massung
 */

#include <iostream>

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

std::vector<std::pair<sentence, double>>
    diff::candidates(const sentence& sent, bool use_lm /* = false */)
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
    if (use_lm)
        step_lm(sent, candidates, 0);
    else
        step(sent, candidates, 0);

    std::vector<pair_t> sorted;
    while (!candidates.empty())
    {
        sorted.emplace_back(std::move(candidates.top()));
        candidates.pop();
    }
    std::reverse(sorted.begin(), sorted.end());
    return sorted;
}

template <class PQ>
void diff::add(PQ& candidates, sentence& sent)
{
    seen_.insert(sent.to_string());
    candidates.emplace(sent, lm_.perplexity_per_word(sent));
    if (candidates.size() > max_cand_size_)
        candidates.pop();
}

template <class PQ>
void diff::remove(const sentence& sent, size_t idx, PQ& candidates,
                  uint64_t depth)
{
    sentence rem_cpy{sent};
    rem_cpy.remove(idx);
    if (seen_.find(rem_cpy.to_string()) == seen_.end())
    {
        add(candidates, rem_cpy);
        step(rem_cpy, candidates, depth + 1);
    }
}

template <class PQ>
void diff::lm_ops(const sentence& sent, size_t idx, PQ& candidates,
                  uint64_t depth, bool substitute)
{
    if (idx < n_val_ - 1)
        return;

    auto spliced = sent(idx - (n_val_ - 1), idx);
    try
    {
        auto best = lm_.top_k(spliced, 5);
        for (auto& p : best)
        {
            if (p.first == "</s>")
                continue;
            sentence cpy{sent};
            if (substitute)
                cpy.insert(idx, p.first);
            else
                cpy.substitute(idx, p.first);
            if (seen_.find(cpy.to_string()) == seen_.end())
            {
                add(candidates, cpy);
                step_lm(cpy, candidates, depth + 1);
            }
        }
    }
    catch (language_model_exception& ex)
    {
        // ignore if there are no transitions found
    }
}

template <class PQ>
void diff::insert(const sentence& sent, size_t idx, PQ& candidates,
                  uint64_t depth)
{
    for (auto& fw : fwords_)
    {
        sentence ins_cpy{sent};
        ins_cpy.insert(idx, fw);
        if (seen_.find(ins_cpy.to_string()) == seen_.end())
        {
            add(candidates, ins_cpy);
            step(ins_cpy, candidates, depth + 1);
        }
    }
}

template <class PQ>
void diff::substitute(const sentence& sent, size_t idx, PQ& candidates,
                      uint64_t depth)
{
    std::string stemmed{sent[idx]};
    Porter2Stemmer::stem(stemmed);
    auto it = stems_.find(stemmed);
    if (it != stems_.end() && it->second.size() != 1)
    {
        for (auto& stem : it->second)
        {
            sentence subbed{sent};
            subbed.substitute(idx, stem);
            if (seen_.find(subbed.to_string()) == seen_.end())
            {
                add(candidates, subbed);
                step(subbed, candidates, depth + 1);
            }
        }
    }
}

template <class PQ>
void diff::step(const sentence& sent, PQ& candidates, size_t depth)
{
    if (depth == max_depth_)
        return;

    for (size_t i = 0; i < sent.size(); ++i)
    {
        remove(sent, i, candidates, depth);
        insert(sent, i, candidates, depth);
        substitute(sent, i, candidates, depth);
    }
}

template <class PQ>
void diff::step_lm(const sentence& sent, PQ& candidates, size_t depth)
{
    if (depth == max_depth_)
        return;

    for (size_t i = 0; i <= sent.size(); ++i)
    {
        remove(sent, i, candidates, depth);
        insert(sent, i, candidates, depth);
        lm_ops(sent, i, candidates, depth, true);
        lm_ops(sent, i, candidates, depth, false);
        substitute(sent, i, candidates, depth);
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
