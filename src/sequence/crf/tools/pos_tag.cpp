/**
 * @file pos_tag.cpp
 * @author Sean Massung
 */

#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/classify/confusion_matrix.h"
#include "cpptoml.h"
#include "meta/logging/logger.h"
#include "meta/sequence/crf/crf.h"
#include "meta/sequence/crf/tagger.h"
#include "meta/sequence/io/ptb_parser.h"
#include "meta/sequence/sequence.h"
#include "meta/sequence/crf/tagger.h"

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
    auto crf_group = config->get_table("crf");
    if (!crf_group)
    {
        std::cerr << "[crf] group needed in config file" << std::endl;
        return 1;
    }

    auto prefix = crf_group->get_as<std::string>("prefix");
    if (!prefix)
    {
        std::cerr << "prefix to learned model needed in [crf] group"
                  << std::endl;
        return 1;
    }

    sequence::crf crf{*prefix};
    auto ana = sequence::default_pos_analyzer();
    ana.load(*prefix);
    const auto& analyzer = ana;
    auto tagger = crf.make_tagger();

    std::string line;
    std::cout << "Type a sentence to have it POS-tagged, blank to exit."
              << std::endl;
    while (true)
    {
        std::cout << " > ";
        std::getline(std::cin, line);

        if (line.empty())
            break;

        std::unique_ptr<analyzers::token_stream> stream
            = make_unique<analyzers::tokenizers::icu_tokenizer>();
        stream->set_content(std::move(line));
        sequence::sequence seq;
        while (*stream)
        {
            auto token = stream->next();
            if (token == " " || token == "<s>" || token == "</s>")
                continue;
            seq.add_observation(
                {sequence::symbol_t{token}, sequence::tag_t{"[UNK]"}});
        }

        analyzer.analyze(seq);
        tagger.tag(seq);
        std::cout << "=> ";
        for (auto& obs : seq)
        {
            std::cout << obs.symbol() << "_" << analyzer.tag(obs.label())
                      << " ";
        }
        std::cout << std::endl;
    }
}
