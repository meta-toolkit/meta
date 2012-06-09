/**
 * @file index.h
 */

#ifndef _INDEX_H_
#define _INDEX_H_

#include <map>
#include <string>
#include "document.h"

using std::multimap;
using std::string;

/**
 * An abstract class that represents searchable document-based index.
 */
class Index
{
    public:

        /**
         * Scores a document given a query.
         * @param document - the doc to score
         * @param query - the query to score against
         * @return the real score value 
         */
        virtual double scoreDocument(const Document & document, const Document & query) const = 0;

        /**
         * @return the average document length of the collection
         */
        virtual size_t getAvgDocLength() const = 0;

        /**
         * Searches the index using the scoreDocument function on each Document.
         * @param query - the query to perform the search with
         * @return - a mapping of scores to Documents
         */
        virtual multimap<double, string> search(const Document & query) const = 0;

        /**
         * Classify the query document by category using K-Nearest Neighbor.
         * @param query - the query to run
         * @param k - the value of k in KNN
         * @return the category the document is believed to be in
         */
        virtual string classifyKNN(const Document & query, size_t k) const = 0;
};

#endif
