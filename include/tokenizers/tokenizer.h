/**
 * @file tokenizer.h
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include "index/structs.h"
#include "util/invertible_map.h"

class Document;

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

        virtual ~Tokenizer() = default;

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        virtual void tokenize(Document & document,
                const std::shared_ptr<std::unordered_map<TermID, unsigned int>> & docFreq = nullptr) = 0;

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

        /**
         *
         */
        InvertibleMap<TermID, std::string> getTermIDMapping() const;

        /**
         * TODO there are probably other functions that MultiTokenizer messes up
         * Looks up the actual label that is represented by a TermID.
         * @param termID
         * @return the label
         */
        virtual std::string getLabel(TermID termID) const;

        /**
         * Prints the data associated with this tokenizer, consisting of a TermID and its string
         * value.
         */
        void printData() const;

        /**
         * @return the number of terms seen so far by this tokenizer
         */
        size_t getNumTerms() const;

        /**
         * Sets the current termID for this tokenizer. This is useful when running multiple
         * tokenizers on a single documents.
         */
        void setMaxTermID(size_t start);

        TermID getMaxTermID() const;
 
    private:

        /** Internal counter for the number of unique terms seen (used as keys
         * in the InvertibleMap) */
        TermID _currentTermID;

        /** Keeps track of the internal mapping of TermIDs to strings parsed
         * from the file */
        InvertibleMap<TermID, std::string> _termMap;
};

#endif
