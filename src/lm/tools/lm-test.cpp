/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "cpptoml.h"
#include "meta.h"
#include "lm/diff.h"
#include "lm/sentence.h"
#include "lm/language_model.h"
#include "logging/logger.h"
#include "util/progress.h"
#include "util/filesystem.h"

using namespace meta;

int main(int argc, char* argv[])
{
    logging::set_cerr_logging();
    lm::language_model model{cpptoml::parse_file(argv[1])};
    lm::sentence s1{"I disagree with this statement for several reasons .",
                    false};
    std::cout << s1.to_string() << ": " << model.log_prob(s1) << std::endl;
    lm::sentence s2{"I disagree with this octopus for several reasons .",
                    false};
    std::cout << s2.to_string() << ": " << model.log_prob(s2) << std::endl;
    lm::sentence s3{"Hello world !", false};
    std::cout << s3.to_string() << ": " << model.log_prob(s3) << std::endl;
    lm::sentence s4{"xyz xyz xyz", false};
    std::cout << s4.to_string() << ": " << model.log_prob(s4) << std::endl;

    /*
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml sentences.txt"
                  << std::endl;
        return 1;
    }

    lm::diff correcter{cpptoml::parse_file(argv[1])};
    std::ifstream in{argv[2]};
    auto num_sentences = filesystem::num_lines(argv[2]);
    printing::progress prog{"Editing sentences ", num_sentences};
    std::ofstream out{std::string{argv[2]} + ".out"};
    std::ofstream log{std::string{argv[2]} + ".log"};
    std::string line;
    size_t done = 0;
    double do_nothing = 0;
    while (in)
    {
        std::getline(in, line);
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
    */
}
