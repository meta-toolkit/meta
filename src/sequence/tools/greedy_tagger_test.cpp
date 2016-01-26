/**
 * @file greedy_tagger_train.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "cpptoml.h"
#include "meta/classify/confusion_matrix.h"
#include "meta/io/filesystem.h"
#include "meta/logging/logger.h"
#include "meta/sequence/perceptron.h"
#include "meta/sequence/io/ptb_parser.h"
#include "meta/util/progress.h"

using namespace meta;

std::string two_digit(uint8_t num)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}

/**
 * For config params, see greedy_tagger_train.
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

    auto seq_grp = config->get_table("sequence");
    if (!seq_grp)
    {
        LOG(fatal) << "Configuration must contain a [sequence] group" << ENDLG;
        return 1;
    }

    auto seq_prefix = seq_grp->get_as<std::string>("prefix");
    if (!seq_prefix)
    {
        LOG(fatal)
            << "[sequence] group must contain a prefix to store model files"
            << ENDLG;
        return 1;
    }

    auto treebank = seq_grp->get_as<std::string>("treebank");
    if (!treebank)
    {
        LOG(fatal) << "[sequence] group must contain a treebank path" << ENDLG;
        return 1;
    }

    auto corpus = seq_grp->get_as<std::string>("corpus");
    if (!corpus)
    {
        LOG(fatal) << "[sequence] group must contain a corpus" << ENDLG;
        return 1;
    }

    auto test_sections = seq_grp->get_array("test-sections");
    if (!test_sections)
    {
        LOG(fatal) << "[sequence] group must contain test-sections" << ENDLG;
        return 1;
    }

    auto section_size = seq_grp->get_as<int64_t>("section-size");
    if (!section_size)
    {
        LOG(fatal) << "[sequence] group must contain section-size" << ENDLG;
        return 1;
    }

    std::string path = *prefix + "/" + *treebank + "/treebank-2/tagged/"
                       + *corpus;

    std::vector<sequence::sequence> testing;
    {
        auto begin = test_sections->at(0)->as<int64_t>()->get();
        auto end = test_sections->at(1)->as<int64_t>()->get();
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
                for (auto& seq : sequences)
                    testing.emplace_back(std::move(seq));
            }
        }
    }

    sequence::perceptron tagger{*seq_prefix};

    // run the tagger on every sequence, measuring statistics for
    // token-level accuracy, F1, etc.
    classify::confusion_matrix matrix;
    {
        printing::progress progress{" > Tagging: ", testing.size()};
        for (uint64_t i = 0; i < testing.size(); ++i)
        {
            progress(i);
            auto& seq = testing[i];
            std::vector<sequence::tag_t> correct;
            for (const auto& obs : seq)
                correct.emplace_back(obs.tag());

            tagger.tag(seq);

            for (uint64_t t = 0; t < seq.size(); ++t)
                matrix.add(predicted_label{seq[t].tag()},
                           class_label{correct[t]});
        }
    }
    matrix.print_stats();

    return 0;
}
