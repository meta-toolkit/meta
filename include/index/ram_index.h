/**
 * @file ram_index.h
 */

#ifndef _RAM_INDEX_
#define _RAM_INDEX_

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
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
        RAMIndex(const std::vector<Document> & indexDocs, std::shared_ptr<Tokenizer> tokenizer);

        /**
         * Creates an Index located in memory.
         * @param indexFiles - files to index
         * @param tokenizer - how to tokenize the indexed files 
         */
        RAMIndex(const std::vector<std::string> & indexFiles, std::shared_ptr<Tokenizer> tokenizer);
        
        /**
         * Creates an index of given documents.
         * @param documents - a vector of documents to make the index out of
         * @param chunkMBSize - the maximum size the postings chunks will be in
         *  memory before they're written to disk.
         */
        void indexDocs(std::vector<Document> & documents, size_t chunkMBSize);

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

    private:

        /** stuff */
        std::shared_ptr<Tokenizer> _tokenizer;

        /** documents stored in this index */
        std::vector<Document> _documents;

        /** the IDF values of all the terms encountered in the document collection */
        std::shared_ptr<std::unordered_map<TermID, unsigned int>> _docFreqs;

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
