/**
 * @file create-dataset.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <fstream>
#include "meta.h"
#include "cpptoml.h"
#include "lm/diff.h"
#include "lm/sentence.h"

using namespace meta;

int main(int argc, char* argv[])
{
    lm::diff correcter{cpptoml::parse_file(argv[1])};
    std::string line;
    std::ifstream in{argv[2]};
    std::ofstream out{"edits.dat"};
    while (in)
    {
        std::getline(in, line);
        if (line.empty())
            continue;
        try
        {
            lm::sentence sent{line};
            auto candidates = correcter.candidates(sent, true);
            auto edits = candidates[0].first.operations();
            if (edits.empty())
                out << "unmodified" << std::endl;
            else
            {
                for (auto& e : edits)
                    out << e << " ";
                out << std::endl;
            }
        }
        catch (lm::sentence_exception& ex)
        {
            out << "error" << std::endl;
        }
    }
}
