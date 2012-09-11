/**
 * @file tokenizer.h
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include "util/invertible_map.h"

class Document;

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
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        virtual void tokenize(Document & document,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq) = 0;

        /**
         * Maps terms to TermIDs.
         * @param term - the term to check
         * @return the TermID assigned to this term
         */
        virtual TermID getMapping(const std::string & term);

        /**
         * Calls the TermID InvertibleMap's saveMap function.
         * @param filename - the filename to save the mapping as
         */
        void saveTermIDMapping(const std::string & filename) const;

        /**
         * Sets the token to termid mapping for this tokenizer.
         * This is useful when reading an inverted index from disk
         *  with an existing mapping.
         * @param mapping - a reference to the desired mapping
         */
        void setTermIDMapping(const InvertibleMap<TermID, std::string> & mapping);

    private:
        
        TermID _currentTermID;
        InvertibleMap<TermID, std::string> _termMap;
};

#endif
