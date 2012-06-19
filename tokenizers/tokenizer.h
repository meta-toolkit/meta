/**
 * @file tokenizer.h
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <unordered_map>
#include "index/document.h"
#include "parse_tree.h"

using std::unordered_map;

typedef unsigned int TermID;

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
         *
         */
        virtual void tokenize(const string & filename, Document & document, unordered_map<TermID, unsigned int>* docFreq);

        /**
         * Maps terms to TermIDs.
         * @param term - the term to check
         * @return the TermID assigned to this term
         */
        virtual TermID getMapping(const string & term);

    private:
        
        TermID _currentTermID;
        unordered_map<string, TermID> _termMap;
};

#endif
