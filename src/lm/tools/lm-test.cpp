/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "meta.h"
#include "lm/language_model.h"
#include "lm/sentence.h"

using namespace meta;

void measure_perplexity(lm::language_model& model, const lm::sentence& line)
{
    std::cout << "=======================================" << std::endl;
    std::cout << "  Sentence: " << line.to_string() << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "Perplexity: " << model.perplexity(line) << std::endl;
    std::cout << "  Per word: " << model.perplexity_per_word(line) << std::endl;
    std::cout << "=======================================" << std::endl;
}

int main(int argc, char* argv[])
{
    lm::language_model model{argv[1], 3};
    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);
        if (line.empty())
            break;
        lm::sentence sent{line};
        measure_perplexity(model, line);
        for (size_t i = 0; i < sent.size(); ++i)
        {
            lm::sentence cpy{sent};
            cpy.remove(i);
            measure_perplexity(model, cpy);
        }
    }
}
