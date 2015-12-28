/**
 * @file diff_test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "cpptoml.h"
#include "meta/meta.h"
#include "meta/lm/diff.h"
#include "meta/lm/sentence.h"
#include "meta/logging/logger.h"
#include "meta/io/filesystem.h"
#include "meta/util/progress.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml sentences.txt"
                  << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    lm::diff correcter{*config};
    std::ifstream in{argv[2]};
    auto num_sentences = filesystem::num_lines(argv[2]);
    printing::progress prog{"Editing sentences ", num_sentences};
    std::ofstream out{std::string{argv[2]} + ".out"};
    std::ofstream log{std::string{argv[2]} + ".log"};
    std::string line;
    size_t done = 0;
    double do_nothing = 0;
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        prog(done++);
        lm::sentence sent{line};
        auto candidates = correcter.candidates(sent, true);
        out << candidates[0].first.to_string() << std::endl;
        log << sent.to_string() << std::endl;
        log << "====================================" << std::endl;
        if (candidates[0].first.operations().empty())
            ++do_nothing;

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
    prog.end();

    std::cout << "Percent no-ops: " << do_nothing / done << std::endl;
}
