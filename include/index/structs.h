/**
 * @file structs.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <iterator>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include "meta.h"

namespace meta {
namespace index {

/**
 * Represents one term's document info.
 */
class PostingData
{
    public:
        /** The numeric id value assigned to this document */
        doc_id docID;

        /** The number of times a term appeared in this document */
        unsigned int freq;

        /** Parameters constructor */
        PostingData(doc_id pdocID, unsigned int pfreq):
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
 * Multiple IndexEntries with the same term_id can be
 *  merged together.
 */
class IndexEntry
{
    public:
        /** denotes which token this entry is for */
        term_id termID;

        /** collection of PostingData for each document this term occurs in */
        std::vector<PostingData> data;

        /**
         * Constructor.
         * @param ptermID - the termID to set
         */
        IndexEntry(term_id ptermID):
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

}
}

#endif
