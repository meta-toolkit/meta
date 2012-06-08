/**
 * @file lexicon.h
 */

#ifndef _LEXICON_H_
#define _LEXICON_H_

#include <unordered_map>
#include <string>

using std::unordered_map;
using std::string;

/**
 * Represents the dictionary or lexicon of an inverted index.
 */
class Lexicon
{
    public:
        /**
         * Constructor to read an existing lexicon from disk.
         */
        Lexicon(const string & lexiconFile);

        /**
         * Constructor to create a new lexicon.
         */
        Lexicon();

    private:
        /**
         * Represents metadata for a specific term in the lexicon.
         */
        class TokenData
        {
            public:
                /**
                 * How many documents this token appears in.
                 */
                size_t numDocs;

                /**
                 * How many times this token appears in total.
                 */
                size_t freq;

                /**
                 * Where to find the token information in the postings file.
                 */
                size_t index;
        };

        unordered_map<string, TokenData> _entries;
};

#endif
