/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "meta.h"
#include "lm/language_model.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    lm::language_model model{argv[1], 3};
    for (size_t i = 1; i < 10; ++i)
    {
        auto sentence = model.generate(i);
        std::cout << sentence << std::endl;
        std::cout << "  -> perplexity_per_word: "
                  << model.perplexity_per_word(sentence) << std::endl;
    }

    std::cout << "Input a sentence to score (blank to quit):" << std::endl;
    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);
        std::cout << model.perplexity_per_word(line) << std::endl;
    }
}
