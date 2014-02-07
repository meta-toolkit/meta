/**
 * @file unit-test.cpp
 */

#include <iostream>
#include <unordered_set>
#include <string>
#include "unit_test.h"
#include "tokenizer_test.h"
#include "inverted_index_test.h"
#include "ranker_test.h"
#include "stemmer_test.h"
#include "forward_index_test.h"
#include "string_list_test.h"
#include "vocabulary_map_test.h"
#include "libsvm_parser_test.h"
#include "classifier_test.h"
#include "parallel_test.h"
#include "ir_eval_test.h"
#include "util/printing.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        std::cerr << "Usage: " << argv[0] << " [opt1 [opt2 [...]]]" << std::endl;
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
        return 1;
    }

    std::unordered_set<std::string> args{argv + 1, argv + argc};
    bool all = args.find("all") != args.end();

    if(all || args.find("tokenizers") != args.end())
    {
        std::cout << printing::make_bold("Testing tokenizers") << std::endl;
        testing::tokenizer_tests();
    }
    if(all || args.find("stemmers") != args.end())
    {
        std::cout << printing::make_bold("Testing stemmers") << std::endl;
        testing::stemmer_tests();
    }
    if(all || args.find("parallel") != args.end())
    {
        std::cout << printing::make_bold("Testing parallel") << std::endl;
        testing::parallel_tests();
    }
    if(all || args.find("inverted-index") != args.end())
    {
        std::cout << printing::make_bold("Testing inverted_index") << std::endl;
        testing::inverted_index_tests();
    }
    if(all || args.find("forward-index") != args.end())
    {
        std::cout << printing::make_bold("Testing forward_index") << std::endl;
        testing::forward_index_tests();
    }
    if(all || args.find("string-list") != args.end())
    {
        std::cout << printing::make_bold("Testing string_list") << std::endl;
        testing::string_list_tests();
    }
    if(all || args.find("vocabulary-map") != args.end())
    {
        std::cout << printing::make_bold("Testing vocabulary_map") << std::endl;
        testing::vocabulary_map_tests();
    }
    if(all || args.find("libsvm-parser") != args.end())
    {
        std::cout << printing::make_bold("Testing libsvm_parser") << std::endl;
        testing::libsvm_parser_tests();
    }
    if(all || args.find("classifiers") != args.end())
    {
        std::cout << printing::make_bold("Testing classifiers") << std::endl;
        testing::classifier_tests();
    }
    if(all || args.find("rankers") != args.end())
    {
        std::cout << printing::make_bold("Testing rankers") << std::endl;
        testing::ranker_tests();
    }
    if(all || args.find("ir-eval") != args.end())
    {
        std::cout << printing::make_bold("Testing IR eval") << std::endl;
        testing::ir_eval_tests();
    }
}
