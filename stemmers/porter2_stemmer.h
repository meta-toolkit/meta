/**
 * @file porter2_stemmer.h
 *
 * Based on
 *  http://snowball.tartarus.org/algorithms/english/stemmer.html
 */

#ifndef _PORTER2_STEMMER_H_
#define _PORTER2_STEMMER_H_

#include <vector>
#include <string>

namespace Porter2Stemmer
{
    std::string stem(const std::string & toStem);

    std::string trim(const std::string & word);

    namespace internal
    {
        std::string finalStem(std::string & word);

        int firstNonVowelAfterVowel(const std::string & word, int start);

        int getStartR1(const std::string & word);

        int getStartR2(const std::string & word, int startR1);

        void changeY(std::string & word);

        void step0(std::string & word);

        bool step1A(std::string & word);

        void step1B(std::string & word, int startR1);

        void step1C(std::string & word);

        void step2(std::string & word, int startR1);

        void step3(std::string & word, int startR1, int startR2);

        void step4(std::string & word, int startR2);

        void step5(std::string & word, int startR1, int startR2);

        inline bool isShort(const std::string & word);

        bool special(std::string & word);

        bool isVowel(char ch);

        bool isVowelY(char ch);

        bool endsWith(const std::string & word, const std::string & str);

        bool endsInDouble(const std::string & word);

        bool replaceIfExists(std::string & word,
            const std::string & suffix, const std::string & replacement,
            size_t start);

        bool isValidLIEnding(char ch);

        bool containsVowel(const std::string & word, size_t start,
                size_t end);
    }
}

#endif
