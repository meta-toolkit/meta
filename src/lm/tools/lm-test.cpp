/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "cpptoml.h"
#include "meta.h"
#include "lm/diff.h"
#include "lm/sentence.h"

using namespace meta;

int main(int argc, char* argv[])
{
    lm::diff correcter{cpptoml::parse_file(argv[1])};
    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);
        if (line.empty())
            break;

        lm::sentence sent{line};
        auto candidates = correcter.candidates(sent, true);
        std::cout << "Found " << candidates.size() << " candidates."
                  << std::endl;

        for (size_t i = 0; i < 5; ++i)
        {
            std::cout << "====================================" << std::endl;
            std::cout << (i + 1) << "." << std::endl;
            std::cout << " Sentence: " << candidates[i].first.to_string()
                      << std::endl;
            std::cout << " Score: " << candidates[i].second << std::endl;
            std::cout << " Edits:" << std::endl;
            for(auto& e: candidates[i].first.operations())
                std::cout << "    " << e << std::endl;
            std::cout << std::endl;
        }
    }
}
