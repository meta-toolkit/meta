/**
 * @file document.h
 */

#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include <utility>
#include <string>
#include <unordered_map>

using std::pair;
using std::make_pair;
using std::string;
using std::unordered_map;

/**
 * Represents an indexed document.
 */
class Document
{
    public:

        /**
         * Constructor.
         * @param author - the author's name
         * @param nationality - the author's nationality
         */
        Document(string author, string nationality);

        /**
         * Increment the count of the specified transition.
         * @param transition - the rule to increment
         * @param amount - the amount to increment by
         */
        void increment(string transition, size_t amount);

        /**
         * @return the name of this Document's author
         */
        string getAuthor() const;

        /**
         * @return the nationality of this Document's author
         */
        string getNationality() const;

        /**
         * @return the total of transitions recorded for this Document.
         * This is not the number of unique transitions.
         */
        size_t getLength() const;

        /**
         * Get the number of occurrences for a particular transition.
         * @param transition - the transition to check
         */
        size_t getFrequency(string transition) const;

        /**
         * @return the map of frequencies for this document.
         */
        const unordered_map<string, size_t> & getFrequencies() const;

    private:

        string _author;
        string _nationality;
        size_t _length;
        unordered_map<string, size_t> _frequencies;
};

#endif
