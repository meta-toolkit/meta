/**
 * @file index.h
 */

#ifndef _INDEX_H_
#define _INDEX_H_

#include <cmath>
#include <map>
#include <string>

using std::multimap;
using std::string;

/**
 *
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
        virtual double scoreDocument(const Document & document,
                                     const Document & query) const = 0;

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
};

#endif
