/**
 * @file query_mph_lm.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/analyzers/tokenizers/whitespace_tokenizer.h"
#include "meta/lm/mph_language_model.h"
#include "meta/logging/logger.h"
#include "meta/util/algorithm.h"

int main(int argc, char** argv)
{
    using namespace meta;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    lm::mph_language_model model{*config};

    // record similar statistics to KenLM
    double total = 0.0;
    double total_oov_only = 0.0;
    uint64_t oov = 0;
    uint64_t tokens = 0;

    bool verbose = argc > 2;
    std::string line;
    while (std::getline(std::cin, line))
    {
        lm::lm_state state{{model.index("<s>")}};
        lm::lm_state state_next;

        util::for_each_token(
            line.begin(), line.end(), " ",
            [&](std::string::iterator begin, std::string::iterator end) {
                auto tok = util::make_string_view(begin, end);
                auto idx = model.index(tok);
                auto score = model.score(state, idx, state_next);
                if (verbose)
                    std::cout << tok << "=" << idx << " "
                              << state_next.previous.size() << " " << score
                              << " ";
                if (idx == model.unk())
                {
                    total_oov_only += score;
                    ++oov;
                }
                total += score;
                state = state_next;
                ++tokens;
            });

        auto idx = model.index("</s>");
        auto score = model.score(state, idx, state_next);
        if (verbose)
            std::cout << "</s>=" << idx << " " << state_next.previous.size()
                      << " " << score << "\n";
        total += score;
        ++tokens;
    }

    std::cout << "Perplexity including OOVs:\t"
              << std::pow(10, -total / static_cast<double>(tokens))
              << "\nPerplexity excluding OOVs:\t"
              << std::pow(10, -(total - total_oov_only)
                                  / static_cast<double>(tokens - oov))
              << "\nOOVs:\t" << oov << "\nTokens:\t" << tokens << std::endl;

    return 0;
}
