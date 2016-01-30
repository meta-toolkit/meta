/**
 * @file sentence_likelihood.cpp
 * @author Sean Massung
 */

#include <stdexcept>

#include "meta/analyzers/all.h"
#include "meta/analyzers/token_stream.h"
#include "cpptoml.h"
#include "meta/lm/language_model.h"
#include "meta/logging/logger.h"
#include "meta/util/time.h"

using namespace meta;

/**
 * Tokenize a line of input, assuming it is one sentence.
 * @param line Line read from stdin
 * @param config Global config file
 * @return a tokenized lm::sentence object
 */
lm::sentence tokenize_sentence(std::string& line, const cpptoml::table& config)
{
    auto group = config.get_table_array("analyzers");
    if (!group)
        throw std::runtime_error{"[[analyzers]] missing from config"};

    auto stream = analyzers::load_filters(config, *(group->get()[0]));
    if (!stream)
        throw std::runtime_error{"could not initialize token stream"};

    lm::sentence sent; // create empty sentence
    stream->set_content(std::move(line));
    while (*stream)
        sent.push_back(stream->next());

    return sent;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    // The LM will binarize the .arpa file if it hasn't been binarized yet.
    lm::language_model model{*config};

    std::string line;
    std::cout << "Input a sentence, (blank) to quit." << std::endl;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line.empty())
            break;

        // In order to ensure accurate perplexity and log probs, the same
        // tokenization must be applied to the input of the .arpa file as well
        // as the sentence creation right now. We assume the analyzer specified
        // in the config file is the same one used to generate the LM.
        auto sent = tokenize_sentence(line, *config);
        std::cout << "Tokenized sentence: " << sent.to_string() << std::endl;

        double perplexity;
        auto time = common::time([&]()
                                 {
                                     perplexity = model.perplexity(sent);
                                 });
        std::cout << "Perplexity per word: " << perplexity << " ("
                  << time.count() << "ms)" << std::endl;

        double log_prob;
        time = common::time([&]()
                            {
                                log_prob = model.log_prob(sent);
                            });
        std::cout << "Log prob: " << log_prob << " (" << time.count() << "ms)"
                  << std::endl << std::endl;
    }
}
