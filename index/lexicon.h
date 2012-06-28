/**
 * @file lexicon.h
 */

#ifndef _LEXICON_H_
#define _LEXICON_H_

#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <string>
#include <utility>
#include <sstream>
#include "document.h"
#include "util/invertible_map.h"
#include "io/parser.h"

using std::ofstream;
using std::vector;
using std::istringstream;
using std::endl;
using std::cerr;
using std::pair;
using std::make_pair;
using std::unordered_map;
using std::string;

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
         * Destructor.
         */
        ~Lexicon();

        /**
         * Copy constructor.
         */
        Lexicon(const Lexicon & other);

        /**
         * @return whether this Lexicon is empty
         */
        bool isEmpty() const;

        /**
         * Assigns the content of one lexicon to another.
         * @param other - the lexicon to copy
         * @return a reference to the copied lexicon
         */
        const Lexicon & operator=(const Lexicon & other);

        /**
         * @return all lexicon information about a specific term.
         */
        TermData getTermInfo(TermID termID) const;

        /**
         * Writes the lexicon to disk.
         */
        void save() const;

        /**
         * Adds a new term to the lexicon.
         */
        void addTerm(TermID term, TermData termData);

        /**
         * @param docID - the id of the document to get the length of
         * @return the length of the parameter document
         */
        unsigned int getDocLength(DocID docID) const;

        /**
         * @return the number of documents in this collection
         */
        unsigned int getNumDocs() const;

        /**
         * @return the average document length in the collection
         */
        double getAvgDocLength() const;

        /**
         * @return the string term associated with the termID
         */
        string getTerm(TermID termID) const;

        /**
         * @return the TermID associated with the given term
         */
        TermID getTermID(string term) const;
        
        /**
         * @return the string document name associated with the termID
         */
        string getDoc(DocID docID) const;
        
        /**
         * @return the DocID of the given document name
         */
        DocID getDocID(string docName) const;

    private:
 
        string _lexiconFilename;
        string _lengthsFilename;

        double _avgDL;
        bool _avgDLSet;

        unordered_map<TermID, TermData>* _entries;
        unordered_map<DocID, unsigned int>* _docLengths;

        InvertibleMap<TermID, string>* _termMap;
        InvertibleMap<DocID, string>* _docMap;

        /**
         * Reads a lexicon from disk if it exists.
         * This function is called from the lexicon constructor as
         *  well as the InvertedIndex constructor.
         */
        void readLexicon();

        /**
         * Reads the document lengths from disk.
         */
        void readDocLengths();

        /**
         * Calculates the average document length of the collection
         *  and stores it in the member variable.
         */
        void setAvgDocLength();

        /**
         * Reads the specified file and initializes the TermID mapping
         *  accordingly.
         * @param filename - where the mapping file is located
         */
        void setTermMap(const string & filename);

        /**
         * Reads the specified file and initializes the DocID mapping
         *  accordingly.
         * @param filename - where the mapping file is located
         */
        void setDocMap(const string & filename);

        /**
         * @return the string representation of an object
         */
        template <class T>
        static string toString(T value);

        /**
         * Helper function for operator= and cctr.
         */
        void clear();

        /**
         * Helper function for operator= and destructor.
         */
        void copy(const Lexicon & other);
};

#endif
