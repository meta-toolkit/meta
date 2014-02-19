/**
 * @file stemmer_test.cpp
 * @author Sean Massung
 */

#include "test/stemmer_test.h"

namespace meta
{
namespace testing
{

template <class Stemmer>
void test_stem(Stemmer& stemmer, std::ifstream& in, bool do_stem)
{
    std::string to_stem;
    std::string stemmed;
    while (in >> to_stem >> stemmed)
    {
        std::string orig{to_stem};
        stemmer(to_stem);
        if (do_stem)
            ASSERT_EQUAL(to_stem, stemmed);
        else
            ASSERT_EQUAL(to_stem, orig);
    }
}

int stemmer_tests()
{
    int num_failed = 0;

    num_failed += testing::run_test("no-stemmer", [&]()
    {
        std::ifstream in{"../data/porter2_stems.txt"};
        stemmers::no_stemmer stemmer;
        test_stem(stemmer, in, false);
    });

    num_failed += testing::run_test("porter2-stemmer", [&]()
    {
        std::ifstream in{"../data/porter2_stems.txt"};
        stemmers::porter2 stemmer;
        test_stem(stemmer, in, true);
    });

    num_failed += testing::run_test("porter2-special-cases", [&]()
    {
        stemmers::porter2 stemmer;
        const static auto unchanged = {"'", "q", "<s>", "</s>"};
        for (auto& w : unchanged)
        {
            std::string to_stem{w};
            stemmer(to_stem);
            ASSERT_EQUAL(to_stem, w);
        }
    });

    testing::report(num_failed);
    return num_failed;
}
}
}
