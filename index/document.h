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

/** Numbering value for Terms in the index */
typedef unsigned int TermID;

/** Numbering value for Documents in the index */
typedef unsigned int DocID;


/**
 * Represents an indexed document.
 */
class Document
{
    public:

        /**
         * Constructor.
         * @param filename - name for the document
         * @param category - a classification category this document belongs to
         */
        Document(string filename, string category);

        /**
         * Increment the count of the specified transition.
         * @param termID - the token count to increment
         * @param amount - the amount to increment by
         */
        void increment(TermID termID, unsigned int amount);

        /**
         * Increment the count of the specified transition.
         * @param termID - the token count to increment
         * @param amount - the amount to increment by
         * @param docFreq - used for IDF
         */
        void increment(TermID termID, unsigned int amount, unordered_map<TermID, unsigned int>* docFreq);

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
         * @param termID - the termID of the term to look up
         */
        size_t getFrequency(TermID termID) const;

        /**
         * @return the map of frequencies for this document.
         */
        const unordered_map<TermID, unsigned int> & getFrequencies() const;

    private:

        string _filename;
        string _category;
        size_t _length;
        unordered_map<TermID, unsigned int> _frequencies;
};

#endif
