/**
 * @file sr_parse.cpp
 * @author Sean Massung
 */

#include "meta/analyzers/filters/all.h"
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/parser/sr_parser.h"
#include "meta/sequence/perceptron.h"
#include "meta/sequence/io/ptb_parser.h"
#include "meta/sequence/sequence.h"
#include "meta/util/shim.h"
#include "cpptoml.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto seq_grp = config->get_table("sequence");
    if (!seq_grp)
        throw std::runtime_error{"[sequence] group needed in config file"};

    auto prefix = seq_grp->get_as<std::string>("prefix");
    if (!prefix)
        throw std::runtime_error{"[sequence] group needs a prefix key"};

    auto parser_grp = config->get_table("parser");
    if (!parser_grp)
        throw std::runtime_error{"[parser] group needed in config file"};

    auto parser_prefix = parser_grp->get_as<std::string>("prefix");
    if (!parser_prefix)
        throw std::runtime_error{"[parser] group needs a prefix key"};

    std::cout << "Loading tagging model" << std::endl;
    sequence::perceptron tagger{*prefix};

    std::cout << "Loading parser model" << std::endl;
    parser::sr_parser parser{*parser_prefix};

    std::unique_ptr<analyzers::token_stream> stream
        = make_unique<analyzers::tokenizers::icu_tokenizer>();
    stream = make_unique<analyzers::filters::ptb_normalizer>(std::move(stream));

    std::string line;
    std::cout << "Type a sentence to have it parsed, blank to exit."
              << std::endl;
    while (true)
    {
        std::cout << " > ";
        std::getline(std::cin, line);

        if (line.empty())
            break;

        sequence::sequence seq;
        stream->set_content(std::move(line));
        while (*stream)
        {
            auto token = stream->next();
            if (token == "<s>")
            {
                seq = {};
            }
            else if (token == "</s>")
            {
                tagger.tag(seq);
                parser.parse(seq).pretty_print(std::cout);
            }
            else
            {
                seq.add_symbol(sequence::symbol_t{token});
            }
        }

        std::cout << std::endl;
    }
}
