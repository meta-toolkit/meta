/**
 * @file ram_index.h
 */

#ifndef _RAM_INDEX_
#define _RAM_INDEX_

#include <cmath>
#include <unordered_map>
#include <map>
#include <omp.h>
#include <string>

#include "index.h"

class Tokenizer;

/**
 * Represents an index that resides in memory and is created on the fly.
 */
class RAMIndex : public Index
{
    public:
        /**
         * Creates an Index located in memory.
         * @param indexDocs - Document objects to index
         * @param tokenizer - how to tokenize the indexed files 
         */
        RAMIndex(const std::vector<Document> & indexDocs, Tokenizer* tokenizer);

        /**
         * Creates an Index located in memory.
         * @param indexFiles - files to index
         * @param tokenizer - how to tokenize the indexed files 
         */
        RAMIndex(const std::vector<string> & indexFiles, Tokenizer* tokenizer);

        /**
         * Scores a document given a query.
         * @param document - the doc to score
         * @param query - the query to score against
         * @return the real score value 
         */
        double scoreDocument(const Document & document, const Document & query) const;

        /**
         * @return the average document length of the collection
         */
        size_t getAvgDocLength() const;

        /**
         * Searches the index using the scoreDocument function on each Document.
         * @param query - the query to perform the search with
         * @return - a mapping of scores to Documents
         */
        std::multimap<double, std::string> search(Document & query) const;

        /**
         * Classify the query document by category using K-Nearest Neighbor.
         * @param query - the query to run
         * @param k - the value of k in KNN
         * @return the category the document is believed to be in
         */
        std::string classifyKNN(Document & query, size_t k) const;

    private:

        /** documents stored in this index */
        std::vector<Document> _documents;

        /** the IDF values of all the terms encountered in the document collection */
        std::unordered_map<TermID, unsigned int> _docFreqs;

        /** average number of terms in the documents in the collection */
        size_t _avgDocLength;

        /**
         * Adds counts to the IDF map.
         * @param newFreqs - the frequencies to add to _docFreqs
         */
        void combineMap(const std::unordered_map<TermID, unsigned int> & newFreqs);

        /**
         * @return a filename minus the path
         */
        std::string shortFilename(const std::string & filename) const;
};

#endif
