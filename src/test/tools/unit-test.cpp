/**
 * @file unit-test.cpp
 */

#include <iostream>
#include <unordered_set>
#include <string>
#include "test/unit_test.h"
#include "test/analyzer_test.h"
#include "test/filter_test.h"
#include "test/inverted_index_test.h"
#include "test/ranker_test.h"
#include "test/stemmer_test.h"
#include "test/forward_index_test.h"
#include "test/string_list_test.h"
#include "test/vocabulary_map_test.h"
#include "test/libsvm_parser_test.h"
#include "test/classifier_test.h"
#include "test/parallel_test.h"
#include "test/ir_eval_test.h"
#include "test/graph_test.h"
#include "test/compression_test.h"
#include "test/parser_test.h"
#include "util/printing.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        std::cerr << "Usage: " << argv[0] << " [opt1 [opt2 [...]]]" << std::endl;
        std::cerr << "where opt is one of: " << std::endl;
        std::cerr << " \"all\": runs all unit tests" << std::endl;
        std::cerr << " \"analyzers\": runs tokenization tests" << std::endl;
        std::cerr << " \"filters\": runs filter tests" << std::endl;
        std::cerr << " \"stemmers\": runs stemmer tests" << std::endl;
        std::cerr << " \"parallel\": runs parallel functionality tests" << std::endl;
        std::cerr << " \"inverted-index\": runs inverted index tests" << std::endl;
        std::cerr << " \"forward-index\": runs forward index tests" << std::endl;
        std::cerr << " \"string-list\": runs string list tests" << std::endl;
        std::cerr << " \"vocabulary-map\": runs vocabulary map tests" << std::endl;
        std::cerr << " \"libsvm-parser\": runs libsvm parser tests" << std::endl;
        std::cerr << " \"classifiers\": runs classifier tests" << std::endl;
        std::cerr << " \"rankers\": runs ranker tests" << std::endl;
        std::cerr << " \"ir-eval\": runs IR evaluation tests" << std::endl;
        std::cerr << " \"compression\": runs compression reading and writing tests" << std::endl;
        std::cerr << " \"graph\": runs undirected and directed graph tests" << std::endl;
        std::cerr << " \"parser\": runs parser tests" << std::endl;
        return 1;
    }

    std::unordered_set<std::string> args{argv + 1, argv + argc};
    bool all = args.find("all") != args.end();
    int num_failed = 0;

    if (all || args.find("analyzers") != args.end())
        num_failed += testing::analyzer_tests();
    if (all || args.find("filters") != args.end())
        num_failed += testing::filter_tests();
    if (all || args.find("stemmers") != args.end())
        num_failed += testing::stemmer_tests();
    if (all || args.find("parallel") != args.end())
        num_failed += testing::parallel_tests();
    if (all || args.find("inverted-index") != args.end())
        num_failed += testing::inverted_index_tests();
    if (all || args.find("forward-index") != args.end())
        num_failed += testing::forward_index_tests();
    if (all || args.find("string-list") != args.end())
        num_failed += testing::string_list_tests();
    if (all || args.find("vocabulary-map") != args.end())
        num_failed += testing::vocabulary_map_tests();
    if (all || args.find("libsvm-parser") != args.end())
        num_failed += testing::libsvm_parser_tests();
    if (all || args.find("classifiers") != args.end())
        num_failed += testing::classifier_tests();
    if (all || args.find("rankers") != args.end())
        num_failed += testing::ranker_tests();
    if (all || args.find("ir-eval") != args.end())
        num_failed += testing::ir_eval_tests();
    if (all || args.find("compression") != args.end())
        num_failed += testing::compression_tests();
    if (all || args.find("graph") != args.end())
        num_failed += testing::graph_tests();
    if (all || args.find("parser") != args.end())
        num_failed += testing::parser_tests();

    return num_failed;
}
