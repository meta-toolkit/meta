/**
 * @file pos_tokenizer.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <unordered_set>
#include "meta/analyzers/filters/ptb_normalizer.h"
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "cpptoml.h"
#include "meta/logging/logger.h"
#include "meta/sequence/crf/crf.h"
#include "meta/sequence/crf/tagger.h"
#include "meta/sequence/io/ptb_parser.h"
#include "meta/sequence/sequence.h"
#include "meta/utf/utf.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    auto keep_list_filename = config->get_as<std::string>("function-words");
    std::unordered_set<std::string> keep_list;
    std::ifstream keep_list_file{*keep_list_filename};
    std::string word;
    while (keep_list_file >> word)
        keep_list.insert(word);

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
    while (std::getline(std::cin, line))
    {
        std::unique_ptr<analyzers::token_stream> stream
            = make_unique<analyzers::tokenizers::icu_tokenizer>();
        stream = make_unique<analyzers::filters::ptb_normalizer>(
            std::move(stream));
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

        if (seq.size() == 0)
            continue;

        std::unordered_set<std::string> ptb_special
            = {"-LRB-", "-RRB-", "-LSB-", "-RSB-", "-LCB-", "-RCB-"};

        analyzer.analyze(seq);
        tagger.tag(seq);
        for (auto& obs : seq)
        {
            auto word = obs.symbol();
            if (ptb_special.find(word) != ptb_special.end())
                std::cout << word << " ";
            else if (keep_list.find(word) != keep_list.end())
                std::cout << utf::foldcase(word) << " ";
            else
                std::cout << analyzer.tag(obs.label()) << " ";
        }
        std::cout << std::endl;
    }
}
