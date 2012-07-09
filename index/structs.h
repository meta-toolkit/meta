/**
 * @file structs.h
 */

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <iterator>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

typedef unsigned int TermID;
typedef unsigned int DocID;

/**
 * Represents one term's document info.
 */
class PostingData
{
    public:
        /** The numeric id value assigned to this document */
        DocID docID;

        /** The number of times a term appeared in this document */
        unsigned int freq;

        /** Parameters constructor */
        PostingData(DocID pdocID, unsigned int pfreq):
            docID(pdocID), freq(pfreq){ /* nothing */ }

        /** No params contructor */
        PostingData():
            docID(0), freq(0){ /* nothing */ }

        /**
         * Compares two PostingDatas.
         * @param other - the PostingData to compare with
         * @return whether this PostingData's docID is less than the parameter 
         */
        bool operator<(const PostingData & other) const;
};

/**
 * Represents metadata for a specific term in the lexicon.
 */
class TermData
{
    public:
        /** The inverse document frequency */
        unsigned int idf;

        /** The total number of occurences of this term */
        unsigned int totalFreq;

        /** The byte address in the inverted index */
        unsigned int postingIndex;

        /** The bit address where this TermData starts */
        unsigned char postingBit;
};

/**
 * Represents one entry in a chunk file.
 * Multiple IndexEntries with the same TermID can be
 *  merged together.
 */
class IndexEntry
{
    public:
        TermID termID;
        std::vector<PostingData> data;

        IndexEntry(TermID ptermID):
            termID(ptermID) { /* nothing */ }

        /**
         * Constructor from a string.
         * @param str - the string to make this IndexEntry from
         */
        IndexEntry(const std::string & str);

        /**
         * @return a string representation of this IndexEntry
         */
        std::string toString() const;

        /**
         * Compares two IndexEntriess.
         * @param other - the IndexEntry to compare with
         * @return whether this IndexEntry's termID is less than the parameter 
         */
        bool operator<(const IndexEntry & other) const;
};

#endif
