/**
 * @file libsvm_parser_test.h
 * @author Sean Massung
 */

#ifndef _META_LIBSVM_PARSER_TEST_H_
#define _META_LIBSVM_PARSER_TEST_H_

#include "unit_test.h"
#include "io/libsvm_parser.h"

namespace meta {
namespace testing {

namespace
{
    void label()
    {
        auto same = { "a b:2e-3 f:4.01 gg:22 h:1",
                      "a  b:2e-3 f:4.01   gg:22 h:1  " };
        for(auto & text: same)
        {
            ASSERT(io::libsvm_parser::label(text) == class_label{"a"});
            auto counts = io::libsvm_parser::counts(text);
            ASSERT(counts.size() == 4);
            ASSERT(counts[0].first  == "b");
            ASSERT(counts[0].second == 2e-3);
            ASSERT(counts[1].first  == "f");
            ASSERT(counts[1].second == 4.01);
            ASSERT(counts[2].first  == "gg");
            ASSERT(counts[2].second == 22.0);
            ASSERT(counts[3].first  == "h");
            ASSERT(counts[3].second == 1.0);
        }
    }

    void no_label()
    {
        auto same = { "bbb:2e-3 f___:4.01 g1g:22 13:1",
                      "bbb:2e-3 f___:4.01   g1g:22 13:1  " };
        for(auto & text: same)
        {
            auto counts = io::libsvm_parser::counts(text, false);
            ASSERT(counts.size() == 4);
            ASSERT(counts[0].first  == "bbb");
            ASSERT(counts[0].second == 2e-3);
            ASSERT(counts[1].first  == "f___");
            ASSERT(counts[1].second == 4.01);
            ASSERT(counts[2].first  == "g1g");
            ASSERT(counts[2].second == 22.0);
            ASSERT(counts[3].first  == "13");
            ASSERT(counts[3].second == 1.0);
        }
    }

    void bad_label()
    {
        auto bad = { "thisdatahasnospaces", "thishasspacelast ", " missing" };
        for(auto & text: bad)
        {
            try
            {
                class_label lbl = io::libsvm_parser::label(text);
                FAIL("An exception was not thrown on invalid input");
            }
            catch (io::libsvm_parser::libsvm_parser_exception ex)
            {
                // nothing, we want an exception!
            }
        }
    }

    void bad_counts()
    {
        auto bad = { "",
                     "lis:uvfs agi uy:",
                     "label :9 5:5",
                     "label 9: 5:5",
                     "label ",
                     "label : :::",
                     "label 9:9 9::9",
                     "label 5:"
        };
        for(auto & text: bad)
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
}

    void libsvm_parser_tests()
    {
        testing::run_test("libsvm-parser-label",      [&]() { label();      });
        testing::run_test("libsvm-parser-no-label",   [&]() { no_label();   });
        testing::run_test("libsvm-parser-bad-label",  [&]() { bad_label();  });
        testing::run_test("libsvm-parser-bad-counts", [&]() { bad_counts(); });
    }
}
}

#endif
