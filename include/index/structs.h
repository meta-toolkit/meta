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

/** Numbering value for Terms in the index */
typedef unsigned int TermID;

/** Numbering value for Documents in the index */
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
        /** denotes which token this entry is for */
        TermID termID;

        /** collection of PostingData for each document this term occurs in */
        std::vector<PostingData> data;

        /**
         * Constructor.
         * @param ptermID - the termID to set
         */
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
