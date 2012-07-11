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

    namespace internal
    {
        std::string finalStem(const std::string & word);

        std::string prepareWord(const std::string & word);

        bool returnImmediately(const std::string & word);

        int getStartR1(const std::string & word);

        int getStartR2(const std::string & word, int startR1);

        void changeY(std::string & word);

        void removeApostrophe(std::string & word);

        void doStep1A(std::string & word);

        void doStep1B(std::string & word, int startR1);

        void doStep1C(std::string & word);

        void doStep2(std::string & word, int startR1);

        void doStep3(std::string & word, int startR1, int startR2);

        void doStep4(std::string & word, int startR2);

        void doStep5(std::string & word, int startR1, int startR2);

        bool isShort(const std::string & word, int startR1);
    }
}

#endif
