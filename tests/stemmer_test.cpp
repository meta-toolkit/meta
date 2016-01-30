/**
 * @file stemmer_test.cpp
 * @author Sean Massung
 */

#include <fstream>

#include "bandit/bandit.h"
#include "meta/analyzers/filters/porter2_stemmer.h"

using namespace bandit;
using namespace meta;

go_bandit([]() {

    describe("[stemmers] porter2 stemmer", []() {

        it("should match given stems", []() {
            std::ifstream in{"../data/porter2_stems.txt"};
            std::string to_stem;
            std::string stemmed;
            while (in >> to_stem >> stemmed) {
                std::string orig{to_stem};
                analyzers::filters::porter2::stem(to_stem);
                AssertThat(to_stem, Equals(stemmed));
            }
        });

        it("should handle special cases", []() {
            const static auto unchanged = {"'", "q", "<s>", "</s>"};
            for (const auto& unchanged_word : unchanged) {
                std::string to_stem{unchanged_word};
                analyzers::filters::porter2::stem(to_stem);
                AssertThat(to_stem, Equals(unchanged_word));
            }
        });
    });
});
