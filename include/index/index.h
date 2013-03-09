/**
 * @file index.h
 */

#ifndef _INDEX_H_
#define _INDEX_H_

#include <map>
#include <vector>
#include <string>

class Document;

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
        virtual std::multimap<double, std::string> search(Document & query) const = 0;

        /**
         * Creates an index of given documents.
         * @param documents - a vector of documents to make the index out of
         * @param chunkMBSize - the maximum size the postings chunks will be in
         *  memory before they're written to disk.
         * @return whether the index creation was successful.
         */
        virtual bool indexDocs(std::vector<Document> & documents, size_t chunkMBSize) = 0;
};

#endif
