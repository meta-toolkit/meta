/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <algorithm>
#include "meta.h"
#include "lm/language_model.h"

using namespace meta;

template <class C>
std::string make_string(const C& cont)
{
    std::string ret = "";
    for (auto& e : cont)
        ret += e + " ";
    return ret;
}

int main(int argc, char* argv[])
{
    lm::language_model model{argv[1], 3};

    std::cout << "Input a sentence to score (blank to quit):" << std::endl;
    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);
        auto lma = model.analysis(line);
        for (auto& p : lma)
            std::cout << make_string(p.first) << ": " << p.second << std::endl;

        std::cout << std::endl;

        using pair_t = decltype(lma[0]);
        auto it = std::min_element(lma.begin(), lma.end(),
            [&](const pair_t& a, const pair_t& b) {
                return a.second < b.second;
            }
        );

        auto bad_one = it->first;
        std::cout << "You might want to modify/remove the word \""
           << bad_one.back() << "\"" << std::endl;

        bad_one.pop_back();
        std::cout << "Candidates:" << std::endl;
        size_t i = 1;
        for(auto& p: model.top_k(bad_one, 5))
        {

            std::cout << " " << i << ". " << p.first << std::endl;
            ++i;
        }
    }
}
