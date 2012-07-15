/**
 * @file porter2_stemmer.h
 *
 * Based on
 *  http://snowball.tartarus.org/algorithms/english/stemmer.html
 * and
 *  https://gist.github.com/2849130
 */

#ifndef _PORTER2_STEMMER_H_
#define _PORTER2_STEMMER_H_

#include <string>

namespace Porter2Stemmer
{
    std::string stem(const std::string & toStem);

    std::string trim(const std::string & word);
    
    namespace internal
    {
        std::string finalStem(std::string & word);

        inline bool returnImmediately(const std::string & word);

        int getStartR1(const std::string & word);

        int getStartR2(const std::string & word, int startR1);

        void changeY(std::string & word);

        void removeApostrophe(std::string & word);

        void step1A(std::string & word);

        void step1B(std::string & word, int startR1);

        void step1C(std::string & word);

        void step2(std::string & word, int startR1);

        void step3(std::string & word, int startR1, int startR2);

        void step4(std::string & word, int startR2);

        void step5(std::string & word, int startR1, int startR2);

        inline bool isShort(const std::string & word, int startR1);
    }
}

#endif
