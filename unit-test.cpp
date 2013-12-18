/**
 * @file unit-test.cpp
 */

#include <iostream>
#include <unordered_set>
#include <string>
#include "unit_test.h"
#include "tokenizer_test.h"
#include "index_test.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        std::cerr << "Usage: " << argv[0] << " [opt1 [opt2 [...]]]" << std::endl;
        std::cerr << "where opt is one of: " << std::endl;
        std::cerr << " \"all\": runs all unit tests" << std::endl;
        std::cerr << " \"tokenizers\": runs tokenization tests" << std::endl;
        std::cerr << " \"indexes\": runs indexing tests" << std::endl;
        return 1;
    }

    std::unordered_set<std::string> args{argv + 1, argv + argc};
    bool all = args.find("all") != args.end();

    if(all || args.find("tokenizers") != args.end())
    {
        std::cout << common::make_bold("Testing tokenizers") << std::endl;
        testing::tokenizer_tests();
    }
    if(all || args.find("indexes") != args.end())
    {
        std::cout << common::make_bold("Testing indexes") << std::endl;
        testing::index_tests();
    }
}
