/**
 * @file stemmer_test.h
 * @author Sean Massung
 */

#ifndef _META_STEMMER_TEST_H_
#define _META_STEMMER_TEST_H_

#include <fstream>

#include "unit_test.h"
#include "stemmers/porter2.h"
#include "stemmers/no_stemmer.h"

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
            ASSERT(to_stem == stemmed);
        else
            ASSERT(to_stem == orig);
    }
}

void stemmer_tests()
{
    testing::run_test("no-stemmer", [&]()
    {
        std::ifstream in{"../data/porter2_stems.txt"};
        stemmers::no_stemmer stemmer;
        test_stem(stemmer, in, false);
    });

    testing::run_test("porter2-stemmer", [&]()
    {
        std::ifstream in{"../data/porter2_stems.txt"};
        stemmers::porter2 stemmer;
        test_stem(stemmer, in, true);
    });

    testing::run_test("porter2-special-cases", [&]()
    {
        stemmers::porter2 stemmer;
        const static auto unchanged = {"'", "q", "<s>", "</s>"};
        for (auto& w : unchanged)
        {
            std::string to_stem{w};
            stemmer(to_stem);
            ASSERT(to_stem == w);
        }
    });
}
}
}

#endif
