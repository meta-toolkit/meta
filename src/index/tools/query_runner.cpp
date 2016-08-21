/**
 * @file query_runner.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <string>
#include <vector>

#include "meta/corpus/document.h"
#include "meta/index/eval/ir_eval.h"
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/util/printing.h"
#include "meta/util/time.h"

using namespace meta;

/**
 * Prints the current search result in a nice, human-readable format with a
 * simple snippet if possible. The snippet is made from the beginning of the
 * "content" metadata field.
 */
template <class Index, class SearchResult>
void print_results(const Index& idx, const SearchResult& result,
                   uint64_t result_num)
{
    std::string path{idx->doc_path(result.d_id)};
    auto output = printing::make_bold(std::to_string(result_num) + ". " + path)
                  + " (score = " + std::to_string(result.score) + ", docid = "
                  + std::to_string(result.d_id) + ")";
    std::cout << output << std::endl;
    auto mdata = idx->metadata(result.d_id);
    if (auto content = mdata.template get<std::string>("content"))
    {
        auto len = std::min(std::string::size_type{77}, content->size());
        std::cout << content->substr(0, len) << "..." << std::endl << std::endl;
    }
}

/**
 * Prints the current search result in TREC format, which can be read by the
 * trec-eval program. A "name" metadata field is required as the document ID.
 */
template <class Index, class SearchResult>
void print_trec(const Index& idx, const SearchResult& result,
                uint64_t result_num, uint64_t q_id)
{
    auto mdata = idx->metadata(result.d_id);
    if (auto name = mdata.template get<std::string>("name"))
    {
        std::cout << q_id << "\t_\t" << *name << "\t" << result_num << "\t"
                  << result.score << "\tMeTA" << std::endl;
    }
    else
        throw std::runtime_error{"\"name\" metadata field is required"};
}

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
        throw std::runtime_error{"\"ranker\" group needed in config"};
    auto ranker = index::make_ranker(*group);

    // Get the config group with options specific to this executable.
    auto query_group = config->get_table("query-runner");
    if (!query_group)
        throw std::runtime_error{"\"query-runner\" group needed in config"};

    // Get the path to the file containing queries
    auto query_path = query_group->get_as<std::string>("query-path");
    if (!query_path)
        throw std::runtime_error{
            "config file needs a \"query-path\" parameter"};
    if (!meta::filesystem::file_exists(*query_path))
        throw std::runtime_error{"query path does not exist: " + *query_path};
    std::ifstream queries{*query_path};

    // Read the rest of the options for this executable.
    auto trec_format = query_group->get_as<bool>("trec-format").value_or(false);
    auto max_results = static_cast<uint64_t>(
        query_group->get_as<int64_t>("max-results").value_or(10));
    auto q_id = static_cast<uint64_t>(
        query_group->get_as<int64_t>("query-id-start").value_or(1));

    // create the IR evaluation scorer if necessary
    std::unique_ptr<index::ir_eval> eval;
    try
    {
        if (!trec_format)
            eval = make_unique<index::ir_eval>(*query_group);
    }
    catch (index::ir_eval_exception& ex)
    {
        LOG(info) << "Could not find relevance judgements; skipping eval"
                  << ENDLG;
    }

    std::string content;
    auto elapsed_seconds = common::time([&]() {
        while (std::getline(queries, content))
        {
            corpus::document query{doc_id{0}};
            query.content(content);
            if (!trec_format)
            {
                std::cout << std::string(80, '=') << std::endl;
                std::cout << "Query " << q_id << ": \"" << content << "\""
                          << std::endl;
                std::cout << std::string(80, '-') << std::endl;
            }
            auto ranking = ranker->score(*idx, query, max_results);
            uint64_t result_num = 1;
            for (auto& result : ranking)
            {
                if (trec_format)
                    print_trec(idx, result, result_num, q_id);
                else
                    print_results(idx, result, result_num);
                if (result_num++ == max_results)
                    break;
            }
            if (!trec_format && eval)
                eval->print_stats(ranking, query_id{q_id});
            ++q_id;
        }
    });

    if (!trec_format && eval)
    {
        std::cout << printing::make_bold("  MAP: ") << eval->map() << std::endl;
        std::cout << printing::make_bold(" gMAP: ") << eval->gmap()
                  << std::endl;
        std::cout << std::endl;
    }
    std::cerr << "Elapsed time: " << elapsed_seconds.count() << "ms"
              << std::endl;
}
