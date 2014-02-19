/**
 * @file unit-test.cpp
 */

#include <iostream>
#include <unordered_set>
#include <string>
#include "test/unit_test.h"
#include "test/tokenizer_test.h"
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
#include "util/printing.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        std::cerr << "Usage: " << argv[0] << " [opt1 [opt2 [...]]] [--debug]" << std::endl;
        std::cerr << "where opt is one of: " << std::endl;
        std::cerr << " \"all\": runs all unit tests" << std::endl;
        std::cerr << " \"tokenizers\": runs tokenization tests" << std::endl;
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
        std::cerr << "Specify --debug to not fork() tests" << std::endl;
        return 1;
    }

    std::unordered_set<std::string> args{argv + 1, argv + argc};
    bool all = args.find("all") != args.end();
    testing::debug = args.find("--debug") != args.end();
    int num_failed = 0;

    if (all || args.find("tokenizers") != args.end())
    {
        std::cout << printing::make_bold("Testing tokenizers") << std::endl;
        num_failed += testing::tokenizer_tests();
    }
    if (all || args.find("stemmers") != args.end())
    {
        std::cout << printing::make_bold("Testing stemmers") << std::endl;
        num_failed += testing::stemmer_tests();
    }
    if (all || args.find("parallel") != args.end())
    {
        std::cout << printing::make_bold("Testing parallel") << std::endl;
        num_failed += testing::parallel_tests();
    }
    if (all || args.find("inverted-index") != args.end())
    {
        std::cout << printing::make_bold("Testing inverted_index") << std::endl;
        num_failed += testing::inverted_index_tests();
    }
    if (all || args.find("forward-index") != args.end())
    {
        std::cout << printing::make_bold("Testing forward_index") << std::endl;
        num_failed += testing::forward_index_tests();
    }
    if (all || args.find("string-list") != args.end())
    {
        std::cout << printing::make_bold("Testing string_list") << std::endl;
        num_failed += testing::string_list_tests();
    }
    if (all || args.find("vocabulary-map") != args.end())
    {
        std::cout << printing::make_bold("Testing vocabulary_map") << std::endl;
        num_failed += testing::vocabulary_map_tests();
    }
    if (all || args.find("libsvm-parser") != args.end())
    {
        std::cout << printing::make_bold("Testing libsvm_parser") << std::endl;
        num_failed += testing::libsvm_parser_tests();
    }
    if (all || args.find("classifiers") != args.end())
    {
        std::cout << printing::make_bold("Testing classifiers") << std::endl;
        num_failed += testing::classifier_tests();
    }
    if (all || args.find("rankers") != args.end())
    {
        std::cout << printing::make_bold("Testing rankers") << std::endl;
        num_failed += testing::ranker_tests();
    }
    if (all || args.find("ir-eval") != args.end())
    {
        std::cout << printing::make_bold("Testing IR eval") << std::endl;
        num_failed += testing::ir_eval_tests();
    }

    testing::report(num_failed, true);
    return num_failed;
}
