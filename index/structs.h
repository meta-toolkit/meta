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
#include "util/common.h"
#include "document.h"

using std::istringstream;
using std::string;
using std::vector;

/**
 * Represents one term's document info.
 */
struct PostingData
{
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
struct TermData
{
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
struct IndexEntry
{
    TermID termID;
    vector<PostingData> data;

    /**
     * Constructor from a string.
     * @param str - the string to make this IndexEntry from
     */
    IndexEntry(const string & str);

    /**
     * @return a string representation of this IndexEntry
     */
    string toString() const;
};

#endif
