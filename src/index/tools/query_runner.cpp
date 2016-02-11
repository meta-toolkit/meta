/**
 * @file query_runner.cpp
 * @author Sean Massung
 */

#include <vector>
#include <string>
#include <iostream>

#include "meta/util/time.h"
#include "meta/util/printing.h"
#include "meta/corpus/document.h"
#include "meta/index/inverted_index.h"
#include "meta/index/eval/ir_eval.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"

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
    auto config = cpptoml::parse_file(argv[1]);
    auto idx = index::make_index<index::inverted_index>(*config);

    // Create a ranking class based on the config file.
    auto group = config->get_table("ranker");
    if (!group)
        throw std::runtime_error{"\"ranker\" group needed in config file!"};
    auto ranker = index::make_ranker(*group);

    // Get the path to the file containing queries
    auto query_path = config->get_as<std::string>("query-path");
    if (!query_path)
        throw std::runtime_error{
            "config file needs a \"query-path\" parameter"};
    std::ifstream queries{*query_path};

    std::unique_ptr<index::ir_eval> eval;
    try
    {
        eval = make_unique<index::ir_eval>(*config);
    }
    catch (index::ir_eval_exception& ex)
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
                std::cout << std::string(80, '=') << std::endl;
                std::cout << "Query " << ++i << ": \"" << content << "\""
                          << std::endl;
                std::cout << std::string(80, '-') << std::endl;

                // Use the ranker to score the query over the index.
                auto ranking = ranker->score(*idx, query);
                auto result_num = 1;
                for (auto& result : ranking)
                {
                    std::string path{idx->doc_path(result.d_id)};
                    auto output = printing::make_bold(std::to_string(result_num)
                                                      + ". " + path)
                                  + " (score = " + std::to_string(result.score)
                                  + ", docid = " + std::to_string(result.d_id)
                                  + ")";
                    std::cout << output << std::endl;
                    auto mdata = idx->metadata(result.d_id);
                    if (auto content = mdata.get<std::string>("content"))
                    {
                        auto len = std::min(77ul, content->size());
                        std::cout << content->substr(0, len) << "..."
                                  << std::endl
                                  << std::endl;
                    }
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
