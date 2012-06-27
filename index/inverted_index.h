/**
 * @file inverted_index.h
 */

#ifndef _INVERTED_INDEX_H_
#define _INVERTED_INDEX_H_

#include <map>
#include <unordered_map>
#include <cmath>
#include <string>
#include "index.h"
#include "tokenizers/tokenizer.h"
#include "document.h"
#include "lexicon.h"
#include "postings.h"

using std::multimap;
using std::unordered_map;
using std::string;

/**
 * Represents an index that resides on disk, in the standard inverted format.
 */
class InvertedIndex : public Index
{
    public:

        /**
         * Constructor.
         * If the lexicon file already exists, it loads it into memory.
         * Otherwise, an empty index is created there.
         * @param lexiconFile - where to find the lexicon
         * @param postingsFile - where to find the postings
         * @param tokenizer - how to tokenize the indexed files
         */
        InvertedIndex(const string & lexiconFile, const string & postingsFile, Tokenizer* tokenizer);

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
        multimap<double, string> search(const Document & query) const;

        /**
         * Classify the query document by category using K-Nearest Neighbor.
         * @param query - the query to run
         * @param k - the value of k in KNN
         * @return the category the document is believed to be in
         */
        string classifyKNN(const Document & query, size_t k) const;

    private:

        Lexicon _lexicon;
        Postings _postings;
        Tokenizer* _tokenizer;
};

#endif
