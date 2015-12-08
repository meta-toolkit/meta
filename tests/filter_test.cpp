/**
 * @file filter_test.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <vector>
#include <iostream>

#include "analyzers/tokenizers/whitespace_tokenizer.h"
#include "analyzers/filters/english_normalizer.h"
#include "bandit/bandit.h"
#include "corpus/document.h"
#include "util/shim.h"

using namespace bandit;
using namespace meta;

namespace {

void check_expected(analyzers::token_stream& filter,
                    std::vector<std::string>& expected) {
    AssertThat(static_cast<bool>(filter), IsTrue());
    for (const auto& s : expected)
        AssertThat(filter.next(), Equals(s));
    AssertThat(static_cast<bool>(filter), IsFalse());
}
}

go_bandit([]() {

    describe("[filters] english_normalizer", []() {

        it("should work on easy sentences", []() {
            using namespace analyzers;
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm
                = make_unique<filters::english_normalizer>(std::move(tok));
            norm->set_content("\"This \t\n\f\ris a quote,'' said Dr. Smith.");
            std::vector<std::string> expected
                = {"``", "This", " ",    "is", " ",  "a", " ", "quote", ",",
                   "''", " ",    "said", " ",  "Dr", ".", " ", "Smith", "."};
            check_expected(*norm, expected);
        });

        it("should work with contractions", []() {
            using namespace analyzers;
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm
                = make_unique<filters::english_normalizer>(std::move(tok));
            norm->set_content(
                "What about when we don't want to knee-jerk? We'll "
                "have to do something.");
            std::vector<std::string> expected
                = {"What", " ",   "about", " ",         "when", " ",
                   "we",   " ",   "don",   "'t",        " ",    "want",
                   " ",    "to",  " ",     "knee-jerk", "?",    " ",
                   "We",   "'ll", " ",     "have",      " ",    "to",
                   " ",    "do",  " ",     "something", "."};
            check_expected(*norm, expected);
        });
    });
});
