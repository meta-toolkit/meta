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
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml sentences.txt"
                  << std::endl;
        return 1;
    }

    lm::diff correcter{cpptoml::parse_file(argv[1])};
    std::ifstream in{argv[2]};
    std::ofstream out{std::string{argv[2]} + ".out"};
    std::ofstream log{std::string{argv[2]} + ".log"};
    std::string line;
    while (in)
    {
        std::getline(in, line);
        if (line.empty())
            continue;

        lm::sentence sent{line};
        auto candidates = correcter.candidates(sent, true);
        out << candidates[0].first.to_string() << std::endl;
        log << sent.to_string() << std::endl;
        log << "====================================" << std::endl;

        for (size_t i = 0; i < 5 && i < candidates.size(); ++i)
        {
            log << (i + 1) << ".";
            log << "\tSentence: " << candidates[i].first.to_string()
                << std::endl;
            log << "\tScore: " << candidates[i].second << std::endl;
            log << "\tEdits:" << std::endl;
            for (auto& e : candidates[i].first.operations())
                log << "\t\t" << e << std::endl;
            log << std::endl;
        }
        log << "====================================" << std::endl;
    }
}
