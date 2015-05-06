/**
 * @file query-runner.cpp
 * @author Sean Massung
 */

#include <vector>
#include <string>
#include <iostream>

#include "util/time.h"
#include "util/printing.h"
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/eval/ir_eval.h"
#include "index/ranker/ranker_factory.h"
#include "parser/analyzers/tree_analyzer.h"
#include "sequence/analyzers/ngram_pos_analyzer.h"

using namespace meta;

/**
 * Demo app to read a file with one query per line and run each query on an
 * inverted index.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    // Log to standard error
    logging::set_cerr_logging();

    // Register additional analyzers
    parser::register_analyzers();
    sequence::register_analyzers();

    // Create an inverted index based on the config file
    auto idx = index::make_index<index::inverted_index>(argv[1]);

    // Create a ranking class based on the config file.
    auto config = cpptoml::parse_file(argv[1]);
    auto group = config.get_table("ranker");
    if (!group)
        throw std::runtime_error{"\"ranker\" group needed in config file!"};
    auto ranker = index::make_ranker(*group);

    // Get the path to the file containing queries
    auto query_path = config.get_as<std::string>("query-path");
    if (!query_path)
        throw std::runtime_error{
            "config file needs a \"query-path\" parameter"};
    std::ifstream queries{*query_path};

    std::unique_ptr<index::ir_eval> eval;
    try
    {
        eval = make_unique<index::ir_eval>(argv[1]);
    }
    catch (index::ir_eval::ir_eval_exception& ex)
    {
        LOG(info) << "Could not find relevance judgements; skipping eval"
                  << ENDLG;
    }

    std::string content;
    auto elapsed_seconds = common::time(
        [&]()
        {
            size_t i = 0;
            while (std::getline(queries, content))
            {
                corpus::document query{doc_id{0}};
                query.content(content);
                std::cout << "Query " << ++i << ": " << std::endl;
                std::cout << std::string(20, '=') << std::endl;

                // Use the ranker to score the query over the index.
                auto ranking = ranker->score(*idx, query);
                auto result_num = 1;
                for (auto& result : ranking)
                {
                    std::cout << result_num << ". "
                              << idx->doc_name(result.d_id) << " "
                              << result.score << std::endl;
                    if (result_num++ == 10)
                        break;
                }
                if (eval)
                    eval->print_stats(ranking, query_id{i - 1});
                std::cout << std::endl;
            }
        });

    if (eval)
    {
        std::cout << printing::make_bold("  MAP: ") << eval->map() << std::endl;
        std::cout << printing::make_bold(" gMAP: ") << eval->gmap()
                  << std::endl;
        std::cout << std::endl;
    }
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "ms"
              << std::endl;
}
