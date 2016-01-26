/**
 * @file tokenizer_filter_test.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <vector>

#include "meta/analyzers/tokenizers/whitespace_tokenizer.h"
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/analyzers/tokenizers/character_tokenizer.h"
#include "meta/analyzers/filters/all.h"
#include "bandit/bandit.h"
#include "meta/corpus/document.h"
#include "create_config.h"
#include "meta/util/shim.h"

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
    using namespace analyzers;
    auto config = tests::create_config("line"); // corpus-type doesn't matter
    filters::sentence_boundary::load_heuristics(*config);

    describe("[tokenizer-filter] alpha_filter", [&]() {
        auto tok = make_unique<tokenizers::whitespace_tokenizer>();
        auto norm = make_unique<filters::alpha_filter>(std::move(tok));

        it("should only let alpha chars through", [&]() {
            norm->set_content("\"This \t\n\f\ris a quote,\" said Dr. Smith.");
            std::vector<std::string> expected
                = {"This", "is", "a", "quote", "said", "Dr", "Smith"};
            check_expected(*norm, expected);
        });

        it("should remove non-alpha chars from inside other strings", [&]() {
            norm->set_content("& a*a &b c& && d");
            std::vector<std::string> expected = {"aa", "b", "c", "d"};
            check_expected(*norm, expected);
        });
    });

    describe("[tokenizer-filter] english_normalizer", [&]() {
        auto tok = make_unique<tokenizers::whitespace_tokenizer>();
        auto norm = make_unique<filters::english_normalizer>(std::move(tok));

        it("should work on easy sentences", [&]() {
            norm->set_content("\"This \t\n\f\ris a quote,'' said Dr. Smith.");
            std::vector<std::string> expected
                = {"``", "This", " ",    "is", " ",  "a", " ", "quote", ",",
                   "''", " ",    "said", " ",  "Dr", ".", " ", "Smith", "."};
            check_expected(*norm, expected);
        });

        it("should work with contractions", [&]() {
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

    describe("[tokenizer-filter] icu_filter", [&]() {

        it("should transliterate characters (katakana-latin)", [&]() {
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm = make_unique<filters::icu_filter>(std::move(tok),
                                                         "Katakana-Latin");
            norm->set_content("キャンパス ハロ");
            std::vector<std::string> expected = {"kyanpasu", " ", "haro"};
            check_expected(*norm, expected);
        });

        it("should transliterate characters (greek-latin)", [&]() {
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm = make_unique<filters::icu_filter>(std::move(tok),
                                                         "Greek-Latin");
            norm->set_content("τί φῄς γραφὴν σέ τις ὡς ἔοικε");
            std::vector<std::string> expected
                = {"tí", " ",   "phḗis", " ",   "graphḕn", " ",    "sé",
                   " ",  "tis", " ",     "hōs", " ",       "éoike"};
            check_expected(*norm, expected);
        });

        it("should fail to create transliterator on bad input", [&]() {
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            AssertThrows(std::runtime_error, make_unique<filters::icu_filter>(
                                                 std::move(tok), "garbage"));
        });
    });

    describe("[tokenizer-filter] length_filter", [&]() {

        it("should check min and max parameters", [&]() {
            auto tok1 = make_unique<tokenizers::whitespace_tokenizer>();
            AssertThrows(
                token_stream_exception,
                make_unique<filters::length_filter>(std::move(tok1), 5, 4));
            auto tok2 = make_unique<tokenizers::whitespace_tokenizer>();
            make_unique<filters::length_filter>(std::move(tok2), 5, 5);
        });

        it("should only allow tokens with length on [min, max]", [&]() {
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm
                = make_unique<filters::length_filter>(std::move(tok), 3, 4);
            norm->set_content("1 22 333 4444 55555 22 333 22 1 4444 55555");
            std::vector<std::string> expected = {"333", "4444", "333", "4444"};
            check_expected(*norm, expected);
        });
    });

    describe("[tokenizer-filter] list_filter", [&]() {
        auto stopwords_file = *config->get_as<std::string>("stop-words");

        it("should be able to accept tokens on the list", [&]() {
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm = make_unique<filters::list_filter>(
                std::move(tok), stopwords_file,
                filters::list_filter::type::ACCEPT);
            norm->set_content("supposedly i am the octopus of the big house");
            std::vector<std::string> expected = {"i", "am", "the", "of", "the"};
            check_expected(*norm, expected);
        });

        it("should be able to reject tokens on the list", [&]() {
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm = make_unique<filters::list_filter>(
                std::move(tok), stopwords_file,
                filters::list_filter::type::REJECT);
            norm->set_content("supposedly i am the octopus of the big house");
            std::vector<std::string> expected
                = {"supposedly", " ", " ", " ",   " ", "octopus",
                   " ",          " ", " ", "big", " ", "house"};
            check_expected(*norm, expected);
        });
    });

    describe("[tokenizer-filter] lowercase_filter", [&]() {

        it("should transform uppercase letters to lowercase letters", [&]() {
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm = make_unique<filters::lowercase_filter>(std::move(tok));
            norm->set_content("A\tweIrd Punctuation casE IS HERE!");
            std::vector<std::string> expected
                = {"a",    "\t", "weird", " ", "punctuation", " ",
                   "case", " ",  "is",    " ", "here!"};
            check_expected(*norm, expected);
        });
    });

    describe("[tokenizer-filter] porter2_filter", [&]() {

        it("should transform tokens to their stems", [&]() {
            auto tok = make_unique<tokenizers::whitespace_tokenizer>();
            auto norm = make_unique<filters::porter2_filter>(std::move(tok));
            norm->set_content("In linguistic morphology and "
                              "information retrieval, stemming");
            // note that the comma on retrieval prevents the word
            // form being
            // stemmed
            std::vector<std::string> expected = {
                "In",     " ", "linguist",   " ", "morpholog", " ", "and", " ",
                "inform", " ", "retrieval,", " ", "stem"};
            check_expected(*norm, expected);
        });
    });

    describe("[tokenizer-filter] ptb_normalizer", [&]() {
        auto tok = make_unique<tokenizers::icu_tokenizer>();
        auto norm = make_unique<filters::ptb_normalizer>(std::move(tok));

        it("should tokenize a simple sentence", [&]() {
            norm->set_content("\"That's OK,\" she (begrudgingly) said.");
            std::vector<std::string> expected = {
                "<s>", "``",    "That",         "'s",    "OK",   ",", "''",
                "she", "-LRB-", "begrudgingly", "-RRB-", "said", ".", "</s>"};
            check_expected(*norm, expected);
        });

        it("should insert keywords for parens and braces", [&]() {
            norm->set_content("[&](){};");
            std::vector<std::string> expected
                = {"<s>",   "-LSB-", "&",     "-RSB-", "-LRB-",
                   "-RRB-", "-LCB-", "-RCB-", ";",     "</s>"};
            check_expected(*norm, expected);
        });
    });

    describe("[tokenizer-filter] sentence_boundary", [&]() {
        std::unique_ptr<token_stream> stream;
        stream = make_unique<tokenizers::whitespace_tokenizer>();
        stream = make_unique<filters::english_normalizer>(std::move(stream));
        stream = make_unique<filters::sentence_boundary>(std::move(stream));

        it("should detect sentence boundaries and insert tags", [&]() {
            stream->set_content("Dr. Bob is angry. His face is "
                                "red, and he yells a lot.");
            std::vector<std::string> expected = {
                "<s>", "Dr",   ".", " ",   "Bob", " ",   "is",   " ", "angry",
                ".",   "</s>", " ", "<s>", "His", " ",   "face", " ", "is",
                " ",   "red",  ",", " ",   "and", " ",   "he",   " ", "yells",
                " ",   "a",    " ", "lot", ".",   "</s>"};
            check_expected(*stream, expected);
        });
    });

    describe("[tokenizer-filter] empty_sentence_filter", [&]() {

        auto stopwords_file = *config->get_as<std::string>("stop-words");
        std::unique_ptr<token_stream> stream;
        stream = make_unique<tokenizers::whitespace_tokenizer>();
        stream = make_unique<filters::english_normalizer>(std::move(stream));
        stream = make_unique<filters::sentence_boundary>(std::move(stream));
        stream = make_unique<filters::lowercase_filter>(std::move(stream));
        stream = make_unique<filters::list_filter>(
            std::move(stream), stopwords_file,
            filters::list_filter::type::REJECT);
        stream = make_unique<filters::empty_sentence_filter>(std::move(stream));
        stream = make_unique<filters::length_filter>(std::move(stream), 2, 35);

        it("should remove empty sentences", [&]() {
            stream->set_content("It. Is. Dumb.");
            std::vector<std::string> expected = {"<s>", "dumb", "</s>"};
            check_expected(*stream, expected);
        });

        it("should have no effect if there are no empty sentences", [&]() {
            stream->set_content("Abcd. Efgh. Ijkl.");
            std::vector<std::string> expected
                = {"<s>",  "abcd", "</s>", "<s>", "efgh",
                   "</s>", "<s>",  "ijkl", "</s>"};
            check_expected(*stream, expected);
        });
    });

    describe("[tokenizer-filter] icu_tokenizer", [&]() {

        it("should tokenize based on the unicode standard", [&]() {
            auto tok = make_unique<tokenizers::icu_tokenizer>();
            tok->set_content("\"Hey, you,\" she said. (What?)");
            std::vector<std::string> expected
                = {"<s>", "\"",   "Hey", ",", "you",  ",", "\"", "she", "said",
                   ".",   "</s>", "<s>", "(", "What", "?", ")",  "</s>"};
            check_expected(*tok, expected);
        });

        it("should be able to suppress sentence tags", [&]() {
            auto tok = make_unique<tokenizers::icu_tokenizer>(true);
            tok->set_content("\"Hey, you,\" she said. (What?)");
            std::vector<std::string> expected
                = {"\"",   "Hey", ",", "you",  ",", "\"", "she",
                   "said", ".",   "(", "What", "?", ")"};
            check_expected(*tok, expected);
        });
    });

    describe("[tokenizer-filter] character_tokenizer", [&]() {

        it("should tokenize each character", [&]() {
            auto tok = make_unique<tokenizers::character_tokenizer>();
            tok->set_content("\"Hey, you,\" she said. (What?)");
            std::vector<std::string> expected
                = {"\"", "H", "e", "y", ",", " ", "y", "o", "u", ",",
                   "\"", " ", "s", "h", "e", " ", "s", "a", "i", "d",
                   ".",  " ", "(", "W", "h", "a", "t", "?", ")"};
            check_expected(*tok, expected);
        });
    });
});
