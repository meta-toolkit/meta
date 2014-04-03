/**
 * @file stemmer_test.cpp
 * @author Sean Massung
 */

#include "porter2_stemmer.h"
#include "test/stemmer_test.h"

namespace meta
{
namespace testing
{

int stemmer_tests()
{
    int num_failed = 0;

    num_failed += testing::run_test("porter2-stemmer", [&]()
    {
        std::ifstream in{"../data/porter2_stems.txt"};
        std::string to_stem;
        std::string stemmed;
        while (in >> to_stem >> stemmed)
        {
            std::string orig{to_stem};
            Porter2Stemmer::stem(to_stem);
            ASSERT_EQUAL(to_stem, stemmed);
        }
    });

    num_failed += testing::run_test("porter2-special-cases", [&]()
    {
        const static auto unchanged = {"'", "q", "<s>", "</s>"};
        for (auto& w : unchanged)
        {
            std::string to_stem{w};
            Porter2Stemmer::stem(to_stem);
            ASSERT_EQUAL(to_stem, w);
        }
    });

    return num_failed;
}
}
}
