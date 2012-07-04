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
         * @param path - the path to the document
         */
        Document(const string & path);

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
         * @return the path to this document (the argument to the constructor)
         */
        string getPath() const;

        /**
         * @return the classification category this document is in
         */
        string getCategory() const;

        /**
         * @return the name of this document
         */
        string getName() const;

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

        string _path;
        string _category;
        string _name;
        size_t _length;
        unordered_map<TermID, unsigned int> _frequencies;

        /**
         * @return the name of a document given its full path
         */
        static string getName(const string & path);

        /**
         * @return the containing directory of a file given its full path
         */
        static string getCategory(const string & path);
};

#endif
