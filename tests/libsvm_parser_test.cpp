/**
 * @file libsvm_parser_test.cpp
 * @author Sean Massung
 */

#include "bandit/bandit.h"
#include "meta/io/libsvm_parser.h"

using namespace bandit;
using namespace meta;

go_bandit([]() {

    describe("[libsvm-parser]", []() {

        it("should parse lines with class labels", []() {
            auto same = {"a 12:2e-3 15:4.01 99:22 122:1",
                         "a  12:2e-3 15:4.01   99:22 122:1  "};
            const double delta = 0.000001;
            for (auto& text : same) {
                AssertThat(io::libsvm_parser::label(text),
                           Equals(class_label{"a"}));
                auto counts = io::libsvm_parser::counts(text);
                AssertThat(counts.size(), Equals(std::size_t{4}));
                AssertThat(counts[0].first, Equals(11ul));
                AssertThat(counts[0].second, EqualsWithDelta(2e-3, delta));
                AssertThat(counts[1].first, Equals(14ul));
                AssertThat(counts[1].second, EqualsWithDelta(4.01, delta));
                AssertThat(counts[2].first, Equals(98ul));
                AssertThat(counts[2].second, EqualsWithDelta(22.0, delta));
                AssertThat(counts[3].first, Equals(121ul));
                AssertThat(counts[3].second, EqualsWithDelta(1.0, delta));
            }
        });

        it("should parse lines without class labels", []() {
            auto same
                = {"1:2e-3 2:4.01 3:22 13:1", "1:2e-3 2:4.01   3:22 13:1  "};
            const double delta = 0.000001;
            for (auto& text : same) {
                auto counts = io::libsvm_parser::counts(text, false);
                AssertThat(counts.size(), Equals(std::size_t{4}));
                AssertThat(counts[0].first, Equals(0ul));
                AssertThat(counts[0].second, EqualsWithDelta(2e-3, delta));
                AssertThat(counts[1].first, Equals(1ul));
                AssertThat(counts[1].second, EqualsWithDelta(4.01, delta));
                AssertThat(counts[2].first, Equals(2ul));
                AssertThat(counts[2].second, EqualsWithDelta(22.0, delta));
                AssertThat(counts[3].first, Equals(12ul));
                AssertThat(counts[3].second, EqualsWithDelta(1.0, delta));
            }
        });

        it("should throw an exception if missing labels", []() {
            AssertThrows(io::libsvm_parser::libsvm_parser_exception,
                         io::libsvm_parser::label(" missing"));
        });

        it("should throw an exception if a line has bad count data", []() {
            using exception = io::libsvm_parser::libsvm_parser_exception;
            using namespace io::libsvm_parser;
            AssertThrows(exception, counts(""));
            AssertThrows(exception, counts("lis:uvfs agi uy:"));
            AssertThrows(exception, counts("label :9 5:5"));
            AssertThrows(exception, counts("label 9: 5:5"));
            AssertThrows(exception, counts("label : :::"));
            AssertThrows(exception, counts("label 9:9 9::9"));
            AssertThrows(exception, counts("label 5:"));
        });
    });
});
