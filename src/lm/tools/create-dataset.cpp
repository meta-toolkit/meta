/**
 * @file create-dataset.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <fstream>
#include "meta.h"
#include "lm/diff.h"
#include "lm/sentence.h"

using namespace meta;

int main(int argc, char* argv[])
{
    lm::diff correcter{argv[1]};
    std::string line;
    std::ifstream in{argv[2]};
    while (in)
    {
        std::getline(in, line);
        if(line.empty())
            continue;
        lm::sentence sent{line};
        auto candidates = correcter.candidates(sent, false);
        std::cout << candidates[0].first.to_string() << std::endl;
        for (auto& e : candidates[0].first.operations())
            std::cout << "  " << e << std::endl;
    }
}
