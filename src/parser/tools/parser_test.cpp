/**
 * @file parser_test.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "cpptoml.h"
#include "meta/io/filesystem.h"
#include "meta/logging/logger.h"
#include "meta/parser/io/ptb_reader.h"
#include "meta/parser/sr_parser.h"
#include "meta/parser/trees/evalb.h"
#include "meta/parser/trees/visitors/empty_remover.h"
#include "meta/parser/sequence_extractor.h"
#include "meta/util/progress.h"

using namespace meta;

std::string two_digit(uint8_t num)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}

/**
 * For config parameters, see parser_train.
 */
int main(int argc, char** argv)
{

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml output_file"
                  << std::endl;
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

    auto parser_grp = config->get_table("parser");
    if (!parser_grp)
    {
        LOG(fatal) << "Configuration must contain a [parser] group" << ENDLG;
        return 1;
    }

    auto parser_prefix = parser_grp->get_as<std::string>("prefix");
    if (!parser_prefix)
    {
        LOG(fatal)
            << "[parser] group must contain a prefix to store model files"
            << ENDLG;
        return 1;
    }

    auto treebank = parser_grp->get_as<std::string>("treebank");
    if (!treebank)
    {
        LOG(fatal) << "[parser] group must contain a treebank path" << ENDLG;
        return 1;
    }

    auto corpus = parser_grp->get_as<std::string>("corpus");
    if (!corpus)
    {
        LOG(fatal) << "[parser] group must contain a corpus" << ENDLG;
        return 1;
    }

    auto test_sections = parser_grp->get_array("test-sections");
    if (!test_sections)
    {
        LOG(fatal) << "[parser] group must contain test-sections" << ENDLG;
        return 1;
    }

    auto section_size = parser_grp->get_as<int64_t>("section-size");
    if (!section_size)
    {
        LOG(fatal) << "[parser] group must contain section-size" << ENDLG;
        return 1;
    }

    std::string path
        = *prefix + "/" + *treebank + "/treebank-3/parsed/mrg/" + *corpus;

    parser::empty_remover transformer;

    std::vector<sequence::sequence> testing;
    std::vector<parser::parse_tree> gold_trees;
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
                auto file = *corpus + "_" + folder + two_digit(j) + ".mrg";
                auto filename = path + "/" + folder + "/" + file;
                auto trees = parser::io::extract_trees(filename);
                for (auto& tree : trees)
                {
                    gold_trees.push_back(tree);
                    parser::sequence_extractor seq_ex;
                    tree.transform(transformer);
                    tree.visit(seq_ex);
                    testing.emplace_back(seq_ex.sequence());
                }
            }
        }
    }
    LOG(info) << testing.size() << " test examples" << ENDLG;

    parser::sr_parser parser{*parser_prefix};

    parser::evalb eval;
    std::ofstream output{argv[2]};
    printing::progress progress{" > Parsing: ", testing.size()};
    for (uint64_t i = 0; i < testing.size(); ++i)
    {
        progress(i);
        auto tree = parser.parse(testing[i]);

        output << tree << "\n";

        eval.add_tree(tree, gold_trees[i]);
    }
    progress.end();

    std::cout << "Matched: " << eval.matched() << "\n"
              << "Gold:    " << eval.gold_total() << "\n"
              << "Test:    " << eval.proposed_total() << std::endl;

    std::cout << "Labeled Recall:    " << eval.labeled_recall() << "\n"
              << "Labeled Precision: " << eval.labeled_precision() << "\n"
              << "Labeled F1:        " << eval.labeled_f1() << "\n"
              << "Perfect Matching:  " << eval.perfect() << "\n"
              << "Average Crossing:  " << eval.average_crossing() << "\n"
              << "Zero crossing:     " << eval.zero_crossing() << std::endl;

    return 0;
}
