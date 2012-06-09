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
         * @param name - name for the document
         * @param category - a classification category this document belongs to
         */
        Document(string name, string category);

        /**
         * Increment the count of the specified transition.
         * @param transition - the rule to increment
         * @param amount - the amount to increment by
         */
        void increment(string transition, size_t amount);

        /**
         * Increment the count of the specified transition.
         * @param transition - the rule to increment
         * @param amount - the amount to increment by
         * @param docFreq - used for IDF
         */
        void increment(string transition, size_t amount, unordered_map<string, size_t>* docFreq);

        /**
         * @return the name of this Document's author
         */
        string getName() const;

        /**
         * @return the nationality of this Document's author
         */
        string getCategory() const;

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

        string _name;
        string _category;
        size_t _length;
        unordered_map<string, size_t> _frequencies;
};

#endif
