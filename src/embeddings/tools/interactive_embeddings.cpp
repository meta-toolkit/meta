/**
 * @file interactive_embeddings.cpp
 * @author Chase Geigle
 *
 * This tool is an interactive demo over learned word embeddings. Each
 * query will be interpreted down to a unit-length vector and the top 100
 * closest embeddings to that query will be printed along with their score.
 */

#include <iostream>
#include <string>

#include "cpptoml.h"
#include "meta/embeddings/word_embeddings.h"
#include "meta/logging/logger.h"

using namespace meta;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto embed_cfg = config->get_table("embeddings");
    if (!embed_cfg)
    {
        std::cerr << "Missing [embeddings] configuration in " << argv[1]
                  << std::endl;
        return 1;
    }

    auto glove = embeddings::load_embeddings(*embed_cfg);

    std::cout << "Enter a query and press enter.\n> " << std::flush;

    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line.empty())
            break;

        auto query = glove.at(line);
        for (const auto& se : glove.top_k(query.v, 10))
        {
            auto term = glove.term(se.e.tid);
            std::cout << term << " (" << se.score << ")\n";
        }
        std::cout << std::endl;

        std::cout << "> " << std::flush;
    }

    return 0;
}
