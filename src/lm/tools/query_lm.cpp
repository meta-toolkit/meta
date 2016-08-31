/**
 * @file query_lm.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/analyzers/tokenizers/whitespace_tokenizer.h"
#include "meta/lm/language_model.h"
#include "meta/lm/mph_language_model.h"
#include "meta/logging/logger.h"
#include "meta/util/algorithm.h"

template <class LanguageModel>
void query_lm(const LanguageModel& model, bool verbose = false)
{
    using namespace meta;
    // record similar statistics to KenLM
    double total = 0.0;
    double total_oov_only = 0.0;
    uint64_t oov = 0;
    uint64_t tokens = 0;

    std::string line;
    while (std::getline(std::cin, line))
    {
        lm::lm_state state{{model.index("<s>")}};
        lm::lm_state state_next;

        util::for_each_token(
            line.begin(), line.end(), " ",
            [&](std::string::iterator begin, std::string::iterator end) {
                std::string tok{begin, end};
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
}


int main(int argc, char** argv)
{
    using namespace meta;

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml (lm|mph)" << std::endl;
        std::cerr << "\tlm: query using probing language model" << std::endl;
        std::cerr << "\tmph: query using mph language model" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();
    bool verbose = argc > 3;
    auto config = cpptoml::parse_file(argv[1]);
    util::string_view type{argv[2]};

    if (type == "lm")
    {
        lm::language_model model{*config};
        query_lm(model, verbose);
    }
    else if (type == "mph")
    {
        lm::mph_language_model model{*config};
        query_lm(model, verbose);
    }
    else
    {
        LOG(fatal) << "Unrecognized language model type" << ENDLG;
        return 1;
    }

    return 0;
}
