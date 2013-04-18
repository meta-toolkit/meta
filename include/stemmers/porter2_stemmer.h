/**
 * @file porter2_stemmer.h
 * @author Sean Massung
 * @date September 2012
 *
 * Implementation of
 * http://snowball.tartarus.org/algorithms/english/stemmer.html
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _PORTER2_STEMMER_H_
#define _PORTER2_STEMMER_H_

#include <vector>
#include <string>

namespace meta {
namespace stemmers {

namespace Porter2Stemmer
{
    std::string stem(const std::string & toStem);

    std::string trim(const std::string & word);

    namespace internal
    {
        size_t firstNonVowelAfterVowel(const std::string & word, size_t start);

        size_t getStartR1(const std::string & word);

        size_t getStartR2(const std::string & word, size_t startR1);

        void changeY(std::string & word);

        void step0(std::string & word);

        bool step1A(std::string & word);

        void step1B(std::string & word, size_t startR1);

        void step1C(std::string & word);

        void step2(std::string & word, size_t startR1);

        void step3(std::string & word, size_t startR1, size_t startR2);

        void step4(std::string & word, size_t startR2);

        void step5(std::string & word, size_t startR1, size_t startR2);

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

        bool containsVowel(const std::string & word, int start, int end);
    }
}

}
}

#endif
