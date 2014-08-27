/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "meta.h"
#include "lm/language_model.h"

using namespace meta;

template <class C>
std::string make_string(const C& cont)
{
    std::string ret = "";
    for(auto& e: cont)
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
        for(auto& p: lma)
            std::cout << make_string(p.first) << ": " << p.second << std::endl;
    }
}
