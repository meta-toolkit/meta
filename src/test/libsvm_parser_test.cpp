/**
 * @file libsvm_parser_test.cpp
 * @author Sean Massung
 */

#include "test/libsvm_parser_test.h"

namespace meta
{
namespace testing
{

void label()
{
    auto same = {"a 12:2e-3 15:4.01 99:22 122:1",
                 "a  12:2e-3 15:4.01   99:22 122:1  "};
    for (auto& text : same)
    {
        ASSERT_EQUAL(io::libsvm_parser::label(text), class_label{"a"});
        auto counts = io::libsvm_parser::counts(text);
        ASSERT_EQUAL(counts.size(), size_t{4});
        ASSERT_EQUAL(counts[0].first, 11ul);
        ASSERT_APPROX_EQUAL(counts[0].second, 2e-3);
        ASSERT_EQUAL(counts[1].first, 14ul);
        ASSERT_APPROX_EQUAL(counts[1].second, 4.01);
        ASSERT_EQUAL(counts[2].first, 98ul);
        ASSERT_APPROX_EQUAL(counts[2].second, 22.0);
        ASSERT_EQUAL(counts[3].first, 121ul);
        ASSERT_APPROX_EQUAL(counts[3].second, 1.0);
    }
}

void no_label()
{
    auto same = {"1:2e-3 2:4.01 3:22 13:1", "1:2e-3 2:4.01   3:22 13:1  "};
    for (auto& text : same)
    {
        auto counts = io::libsvm_parser::counts(text, false);
        ASSERT_EQUAL(counts.size(), size_t{4});
        ASSERT_EQUAL(counts[0].first, 0ul);
        ASSERT_APPROX_EQUAL(counts[0].second, 2e-3);
        ASSERT_EQUAL(counts[1].first, 1ul);
        ASSERT_APPROX_EQUAL(counts[1].second, 4.01);
        ASSERT_EQUAL(counts[2].first, 2ul);
        ASSERT_APPROX_EQUAL(counts[2].second, 22.0);
        ASSERT_EQUAL(counts[3].first, 12ul);
        ASSERT_APPROX_EQUAL(counts[3].second, 1.0);
    }
}

void missing_label()
{
    try
    {
        class_label lbl = io::libsvm_parser::label(" missing");
        FAIL("An exception was not thrown on invalid input");
    }
    catch (io::libsvm_parser::libsvm_parser_exception ex)
    {
        // nothing, we want an exception!
    }
}

void bad_counts()
{
    auto bad = {"",       "lis:uvfs agi uy:", "label :9 5:5",   "label 9: 5:5",
                "label : :::",      "label 9:9 9::9", "label 5:"};
    for (auto& text : bad)
    {
        try
        {
            auto counts = io::libsvm_parser::counts(text);
            std::cout << text << std::endl;
            FAIL("An exception was not thrown on invalid input");
        }
        catch (io::libsvm_parser::libsvm_parser_exception ex)
        {
            // nothing, we want an exception!
        }
    }
}

int libsvm_parser_tests()
{
    int num_failed = 0;

    num_failed += testing::run_test("libsvm-parser-label", [&]()
        { label(); });
    num_failed += testing::run_test("libsvm-parser-no-label", [&]()
        { no_label(); });
    num_failed += testing::run_test("libsvm-parser-bad-label", [&]()
        { missing_label(); });
    num_failed += testing::run_test("libsvm-parser-bad-counts", [&]()
        { bad_counts(); });

    return num_failed;
}
}
}
