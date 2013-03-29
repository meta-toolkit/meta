/**
 * @file porter2_stemmer_test.h
 * Tests functionality of porter2_stemmer namespace.
 */

#ifndef _PORTER2_TESTS_H_
#define _PORTER2_TESTS_H_

#include <fstream>
#include <string>
#include <iostream>
#include "stemmers/porter2_stemmer.h"
#include "test/unit_test.h"

namespace unit_tests
{
    /**
     * Tests stemming with the Porter2 stemmer.
     */
    namespace porter2_stemmer_tests
    {
        using std::string;
        using namespace meta::stemmers;

        void correctStem()
        {
            std::ifstream in("data/diffs.txt");
            string toStem, stemmed;
            while(in >> toStem >> stemmed)
                ASSERT(Porter2Stemmer::stem(toStem) == stemmed);
            PASS;
        }

        void emptyStem()
        {
            ASSERT(Porter2Stemmer::stem("") == "");
            ASSERT(Porter2Stemmer::stem("7") == "7");
            PASS;
        }

        void trim()
        {
            string toTrim = "$tr*imMe_";
            ASSERT(Porter2Stemmer::trim(toTrim) == "trimme");

            toTrim = "'trimMe'"; 
            ASSERT(Porter2Stemmer::trim(toTrim) == "'trimme'");

            toTrim = "*&^!%#";
            ASSERT(Porter2Stemmer::trim(toTrim) == "");

            toTrim = "*&%4'13";
            ASSERT(Porter2Stemmer::trim(toTrim) == "'");

            PASS;
        }
    }
}

#endif
