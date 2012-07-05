/**
 * @file tokenizer.h
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <fstream>
#include <unordered_map>
#include "index/document.h"
#include "parse_tree.h"

using std::ofstream;
using std::unordered_map;

/**
 * An class that provides a framework to produce tokens.
 */
class Tokenizer
{
    public:

        /**
         * Constructor. Simply initializes some member variables that
         *  keep track of the TermID mapping.
         */
        Tokenizer();

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        virtual void tokenize(Document & document, unordered_map<TermID, unsigned int>* docFreq);

        /**
         * Maps terms to TermIDs.
         * @param term - the term to check
         * @return the TermID assigned to this term
         */
        virtual TermID getMapping(const string & term);

        /**
         * Calls the TermID InvertibleMap's saveMap function.
         * @param filename - the filename to save the mapping as
         */
        void saveTermIDMapping(const string & filename) const;

    private:
        
        TermID _currentTermID;
        unordered_map<string, TermID> _termMap;
};

#endif
