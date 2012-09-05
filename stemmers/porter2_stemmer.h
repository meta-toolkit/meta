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
        /*
        class Replacement
        {
            public:
                std::string searchStr;
                std::string replaceStr;
                Replacement(std::string ss, std::string rs):
                    searchStr(ss), replaceStr(rs) { }
        };
        */

        std::string finalStem(std::string & word);

        int firstNonVowelAfterVowel(const std::string & word, int start);

        inline bool returnImmediately(const std::string & word);

        int getStartR1(const std::string & word);

        int getStartR2(const std::string & word, int startR1);

        void changeY(std::string & word);

        void removeApostrophe(std::string & word);

        bool step1A(std::string & word);

        void step1B(std::string & word, int startR1);

        void step1C(std::string & word);

        void step2(std::string & word, int startR1);

        void step3(std::string & word, int startR1, int startR2);

        void step4(std::string & word, int startR2);

        void step5(std::string & word, int startR1, int startR2);

        inline bool isShort(const std::string & word, int startR1);

        bool special(std::string & word);

        /*
        bool replace(const std::vector<Replacement> & replacements,
                     std::string & word, int position);
        */

        bool isVowel(char ch);

        bool isVowelY(char ch);

        bool endsWith(const std::string & word, const std::string & str);

        bool endsInDouble(const std::string & word);

        bool replaceIfExists(std::string & word,
            const std::string & suffix, const std::string & replacement);

        bool validLIEnding(char ch);
    }
}

#endif
