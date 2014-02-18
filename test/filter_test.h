/**
 * @file filter_test.h
 * @author Chase Geigle
 */

#ifndef _FILTER_TEST_H_
#define _FILTER_TEST_H_

#include <vector>
#include <iostream>

#include "analyzers/tokenizers/whitespace_tokenizer.h"
#include "analyzers/filters/english_normalizer.h"
#include "corpus/document.h"

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
        corpus::document d{"[no path]", doc_id{0}};
        d.set_content("\"This \t\n\f\ris a quote,'' said Dr. Smith.");
        analyzers::whitespace_tokenizer tok{d};
        analyzers::english_normalizer norm{tok};

        std::vector<std::string> expected = {
            "``", "This", " ",    "is", " ",  "a", " ", "quote", ",",
            "''", " ",    "said", " ",  "Dr", ".", " ", "Smith", "."};

        check_expected(norm, expected);
    });

    num_failed += testing::run_test("english_normalizer_test_contraction", []()
    {
        corpus::document d{"[no path]", doc_id{0}};
        d.set_content("What about when we don't want to knee-jerk? We'll have to do something.");
        analyzers::whitespace_tokenizer tok{d};
        analyzers::english_normalizer norm{tok};

        std::vector<std::string> expected = {
            "What", " ",   "about", " ",         "when", " ",
            "we",   " ",   "don",   "'t",        " ",    "want",
            " ",    "to",  " ",     "knee-jerk", "?",    " ",
            "We",   "'ll", " ",     "have",      " ",    "to",
            " ",    "do",  " ",     "something", "."};

        check_expected(norm, expected);
    });

    testing::report(num_failed);
    return num_failed;
}


}
}

#endif
