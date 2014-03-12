/**
 * @file filter_test.cpp
 * @author Chase Geigle
 */

#include <vector>
#include <iostream>

#include "analyzers/tokenizers/whitespace_tokenizer.h"
#include "analyzers/filters/english_normalizer.h"
#include "corpus/document.h"
#include "util/shim.h"
#include "test/filter_test.h"
#include "test/unit_test.h"

namespace meta
{
namespace testing
{
namespace
{

void check_expected(analyzers::token_stream& filter,
                    std::vector<std::string>& expected)
{
    ASSERT(filter);
    for (const auto& s : expected)
        ASSERT(filter.next() == s);
    ASSERT(!filter);
}
}

int filter_tests()
{
    int num_failed = 0;

    num_failed += testing::run_test("english_normalizer_test_basic", []()
    {
        using namespace analyzers;
        auto tok = make_unique<tokenizers::whitespace_tokenizer>();
        auto norm = make_unique<filters::english_normalizer>(std::move(tok));
        norm->set_content("\"This \t\n\f\ris a quote,'' said Dr. Smith.");

        std::vector<std::string> expected
            = {"``", "This", " ",    "is", " ",  "a", " ", "quote", ",",
               "''", " ",    "said", " ",  "Dr", ".", " ", "Smith", "."};

        check_expected(*norm, expected);
    });

    num_failed += testing::run_test("english_normalizer_test_contraction", []()
    {
        using namespace analyzers;
        auto tok = make_unique<tokenizers::whitespace_tokenizer>();
        auto norm = make_unique<filters::english_normalizer>(std::move(tok));
        norm->set_content("What about when we don't want to knee-jerk? We'll "
                          "have to do something.");

        std::vector<std::string> expected
            = {"What", " ",   "about", " ",         "when", " ",
               "we",   " ",   "don",   "'t",        " ",    "want",
               " ",    "to",  " ",     "knee-jerk", "?",    " ",
               "We",   "'ll", " ",     "have",      " ",    "to",
               " ",    "do",  " ",     "something", "."};

        check_expected(*norm, expected);
    });

    return num_failed;
}
}
}
