/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <queue>
#include "meta.h"
#include "cpptoml.h"
#include "lm/language_model.h"
#include "lm/sentence.h"

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

template <class PQ>
void step(const lm::sentence& sent, const lm::language_model& model,
          PQ& candidates, const std::vector<std::string>& fwords, size_t depth)
{
    if (depth == 2)
        return;

    for (size_t i = 0; i < sent.size(); ++i)
    {
        lm::sentence rem_cpy{sent};
        rem_cpy.remove(i);
        candidates.emplace(rem_cpy, model.perplexity_per_word(rem_cpy));
        if (candidates.size() > 100)
            candidates.pop();
        step(rem_cpy, model, candidates, fwords, depth + 1);

        for (auto& fw : fwords)
        {
            lm::sentence ins_cpy{sent};
            ins_cpy.insert(i, fw);
            candidates.emplace(ins_cpy, model.perplexity_per_word(ins_cpy));
            if (candidates.size() > 100)
                candidates.pop();
            step(ins_cpy, model, candidates, fwords, depth + 1);
        }
    }
}

int main(int argc, char* argv[])
{
    lm::language_model model{argv[1], 3};
    std::string line;
    using pair_t = std::pair<lm::sentence, double>;
    auto fwords = function_words(argv[1]);
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
        step(sent, model, candidates, fwords, 0);

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
