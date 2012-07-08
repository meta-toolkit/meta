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
         * Searches the index using the scoreDocument function on each Document.
         * @param query - the query to perform the search with
         * @return - a mapping of scores to Documents
         */
        virtual multimap<double, string> search(Document & query) const = 0;

        /**
         * Classify the query document by category using K-Nearest Neighbor.
         * @param query - the query to run
         * @param k - the value of k in KNN
         * @return the category the document is believed to be in
         */
        virtual string classifyKNN(Document & query, size_t k) const = 0;
};

#endif
