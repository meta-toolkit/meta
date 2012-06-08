/**
 * @file ram_index.h
 */

#ifndef _RAM_INDEX_
#define _RAM_INDEX_

#include <cmath>
#include <omp.h>
#include <string>

#include "document.h"
#include "tokenizer.h"
#include "index.h"

using std::string;

/**
 * Represents an index that resides in memory and is created on the fly.
 */
class RAMIndex : public Index
{
    public:

        /**
         * Creates an Index located in memory.
         * @param indexFiles - files to index
         * @param tokenizer - how to tokenize the indexed files 
         */
        RAMIndex(const vector<string> & indexFiles,
                 const Tokenizer* tokenizer);

        /**
         * Scores a document given a query.
         * @param document - the doc to score
         * @param query - the query to score against
         * @return the real score value 
         */
        double scoreDocument(const Document & document,
                             const Document & query) const;

        /**
         * @return the average document length of the collection
         */
        size_t getAvgDocLength() const;

        /**
         * Searches the index using the scoreDocument function on each Document.
         * @param query - the query to perform the search with
         * @return - a mapping of scores to Documents
         */
        multimap<double, string> search(const Document & query) const;

        /**
         * Classify the query document by category using K-Nearest Neighbor.
         * @param query - the query to run
         * @param k - the value of k in KNN
         * @return the category the document is believed to be in
         */
        string classifyKNN(const Document & query, size_t k) const;

        /**
         * @return the name of a document given its full path
         */
        static string getName(const string & path);

        /**
         * @return the containing directory of a file given its full path
         */
        static string getCategory(const string & path);

    private:

        vector<Document> _documents;
        unordered_map<string, size_t> _docFreqs;
        size_t _avgDocLength;

        /**
         *
         */
        string shortFilename(const string & filename) const;
};

#endif
