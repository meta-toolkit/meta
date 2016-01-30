#include <iostream>

#include "cpptoml.h"
#include "meta/classify/confusion_matrix.h"
#include "meta/logging/logger.h"
#include "meta/sequence/crf/crf.h"
#include "meta/sequence/crf/tagger.h"
#include "meta/sequence/io/ptb_parser.h"

using namespace meta;

std::string two_digit(uint8_t num)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}

/**
 * For config params, see crf_train.
 */
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    auto prefix = config->get_as<std::string>("prefix");
    if (!prefix)
    {
        LOG(fatal) << "Global configuration must have a prefix key" << ENDLG;
        return 1;
    }

    auto crf_grp = config->get_table("crf");
    if (!crf_grp)
    {
        LOG(fatal) << "Configuration must contain a [crf] group" << ENDLG;
        return 1;
    }

    auto crf_prefix = crf_grp->get_as<std::string>("prefix");
    if (!crf_prefix)
    {
        LOG(fatal) << "[crf] group must contain a prefix to store model files"
                   << ENDLG;
        return 1;
    }

    auto treebank = crf_grp->get_as<std::string>("treebank");
    if (!treebank)
    {
        LOG(fatal) << "[crf] group must contain a treebank path" << ENDLG;
        return 1;
    }

    auto corpus = crf_grp->get_as<std::string>("corpus");
    if (!corpus)
    {
        LOG(fatal) << "[crf] group must contain a corpus" << ENDLG;
        return 1;
    }

    auto train_sections = crf_grp->get_array("test-sections");
    if (!train_sections)
    {
        LOG(fatal) << "[crf] group must contain test-sections" << ENDLG;
        return 1;
    }

    auto section_size = crf_grp->get_as<int64_t>("section-size");
    if (!section_size)
    {
        LOG(fatal) << "[crf] group must contain section-size" << ENDLG;
        return 1;
    }

    std::string path =
        *prefix + "/" + *treebank + "/treebank-2/tagged/" + *corpus;

    std::vector<sequence::sequence> testing;
    {
        auto begin = train_sections->at(0)->as<int64_t>()->get();
        auto end = train_sections->at(1)->as<int64_t>()->get();
        printing::progress progress(
            " > Reading training data: ",
            static_cast<uint64_t>((end - begin + 1) * *section_size));
        for (auto i = static_cast<uint8_t>(begin); i <= end; ++i)
        {
            auto folder = two_digit(i);
            for (uint8_t j = 0; j <= *section_size; ++j)
            {
                progress(static_cast<uint64_t>(i - begin) * 99 + j);
                auto file = *corpus + "_" + folder + two_digit(j) + ".pos";
                auto filename = path + "/" + folder + "/" + file;
                auto sequences = sequence::extract_sequences(filename);
                for (auto & seq : sequences)
                    testing.emplace_back(std::move(seq));
            }
        }
    }

    auto analyzer = sequence::default_pos_analyzer();
    analyzer.load(*crf_prefix);
    {
        // const is *super important* here! This signals that we want the
        // analyzer to be analyzing in "test mode", meaning that it will not
        // generate new feature_ids while analyzing the sequences. This is
        // exactly what we want when running a CRF to actually perform tagging.

        const auto& ana = analyzer;
        printing::progress progress{" > Generating features: ",
                                    testing.size()};
        uint64_t idx = 0;
        for (auto& seq : testing)
        {
            progress(++idx);
            ana.analyze(seq);
        }
    }

    // load the model
    sequence::crf crf{*crf_prefix};

    // make a tagger
    auto tagger = crf.make_tagger();

    // run the tagger on every sequence, measuring statistics for
    // token-level accuracy, F1, etc.
    classify::confusion_matrix matrix;
    for (auto& seq : testing)
    {
        tagger.tag(seq);
        for (const auto& obs : seq)
        {
            auto tag = analyzer.tag(obs.label());
            matrix.add(predicted_label{tag}, class_label{obs.tag()});
        }
    }
    matrix.print();
    matrix.print_stats();

    return 0;
}
