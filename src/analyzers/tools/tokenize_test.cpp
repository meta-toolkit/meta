/**
 * @file tokenize_test.cpp
 * @author Chase Geigle
 */

#include "cpptoml.h"
#include "meta/analyzers/all.h"
#include "meta/analyzers/token_stream.h"
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

    std::unique_ptr<analyzers::token_stream> stream;

    auto analyzers = config->get_table_array("analyzers");
    for (const auto& group : analyzers->get())
    {
        auto method = group->get_as<std::string>("method");
        if (!method)
            continue;

        if (*method == analyzers::ngram_word_analyzer::id)
        {
            stream = analyzers::load_filters(*config, *group);
            break;
        }
    }

    if (!stream)
    {
        LOG(fatal) << "Failed to find an ngram-word analyzer configuration in "
                   << argv[1] << ENDLG;
        return 1;
    }

    std::cout
        << "Type sentences to be tokenized. Hit enter with no text to exit.\n"
        << std::endl;

    std::cout << "> ";
    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line.empty())
            break;

        stream->set_content(std::move(line));
        while (*stream)
        {
            std::cout << stream->next();
            if (*stream)
                std::cout << " ";
        }
        std::cout << std::endl;
        std::cout << "> ";
    }

    return 0;
}
