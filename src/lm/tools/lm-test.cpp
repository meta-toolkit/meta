/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <algorithm>
#include <queue>
#include "meta.h"
#include "cpptoml.h"
#include "lm/language_model.h"
#include "lm/sentence.h"
#include "porter2_stemmer.h"

using namespace meta;

std::vector<std::string> function_words(const std::string& config_file)
{
    auto config = cpptoml::parse_file(config_file);
    std::ifstream in{*config.get_as<std::string>("function-words")};
    std::vector<std::string> words;
    std::string word;
    while (in >> word)
        words.push_back(word);
    return words;
}

std::unordered_map<std::string, std::vector<std::string>>
    get_stems(const std::string& config_file)
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

    std::unordered_map<std::string, std::vector<std::string>> stems;
    for (auto& t : vocab)
    {
        std::string stemmed{t};
        Porter2Stemmer::stem(stemmed);
        stems[stemmed].push_back(t);
    }

    return stems;
}

template <class PQ>
void
    step(const lm::sentence& sent, const lm::language_model& model,
         PQ& candidates, const std::vector<std::string>& fwords, size_t depth,
         std::unordered_set<std::string>& seen,
         const std::unordered_map<std::string, std::vector<std::string>>& stems)
{
    if (depth == 2 || seen.find(sent.to_string()) != seen.end())
        return;

    for (size_t i = 0; i < sent.size(); ++i)
    {
        // remove

        lm::sentence rem_cpy{sent};
        rem_cpy.remove(i);
        if (seen.find(rem_cpy.to_string()) == seen.end())
        {
            seen.insert(rem_cpy.to_string());
            candidates.emplace(rem_cpy, model.perplexity_per_word(rem_cpy));
            step(rem_cpy, model, candidates, fwords, depth + 1, seen, stems);
        }

        // insert

        for (auto& fw : fwords)
        {
            lm::sentence ins_cpy{sent};
            ins_cpy.insert(i, fw);
            if (seen.find(ins_cpy.to_string()) == seen.end())
            {
                seen.insert(ins_cpy.to_string());
                candidates.emplace(ins_cpy, model.perplexity_per_word(ins_cpy));
                step(ins_cpy, model, candidates, fwords, depth + 1, seen,
                     stems);
            }
        }

        // substitute

        std::string stemmed = sent[i];
        Porter2Stemmer::stem(stemmed);
        auto it = stems.find(stemmed);
        if (it != stems.end() && it->second.size() != 1)
        {
            for (auto& stem : it->second)
            {
                lm::sentence subbed{sent};
                subbed.substitute(i, stem);
                if (seen.find(subbed.to_string()) == seen.end())
                {
                    seen.insert(subbed.to_string());
                    candidates.emplace(subbed,
                                       model.perplexity_per_word(subbed));
                    step(subbed, model, candidates, fwords, depth + 1, seen,
                         stems);
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
    lm::language_model model{argv[1], 3};
    std::string line;
    using pair_t = std::pair<lm::sentence, double>;
    auto fwords = function_words(argv[1]);
    auto stems = get_stems(argv[1]);
    auto comp = [](const pair_t& a, const pair_t& b)
    {
        return a.second < b.second;
    };

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);
        if (line.empty())
            break;

        std::priority_queue<pair_t, std::vector<pair_t>, decltype(comp)>
            candidates{comp};
        lm::sentence sent{line};

        candidates.emplace(sent, model.perplexity_per_word(sent));
        std::unordered_set<std::string> seen;
        step(sent, model, candidates, fwords, 0, seen, stems);

        std::cout << "Found " << candidates.size() << " candidates."
                  << std::endl;

        std::vector<pair_t> sorted;
        while (!candidates.empty())
        {
            sorted.push_back(candidates.top());
            candidates.pop();
        }
        std::reverse(sorted.begin(), sorted.end());

        for (size_t i = 0; i < 5; ++i)
        {
            std::cout << "====================================" << std::endl;
            std::cout << (i + 1) << "." << std::endl;
            std::cout << " Sentence: " << sorted[i].first.to_string()
                      << std::endl;
            std::cout << "      PPW: " << sorted[i].second << std::endl;
            std::cout << std::endl;
        }
    }
}
