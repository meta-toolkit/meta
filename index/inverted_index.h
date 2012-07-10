/**
 * @file inverted_index.h
 */

#ifndef _INVERTED_INDEX_H_
#define _INVERTED_INDEX_H_

#include <map>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <string>

#include "index.h"
#include "postings.h"

class Tokenizer;
class Document;
class Postings;
class Lexicon;

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
        InvertedIndex(const std::string & lexiconFile, const std::string & postingsFile, Tokenizer* tokenizer);

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
         * Creates an index of given documents.
         * @param documents - a vector of documents to make the index out of
         * @param chunkMBSize - the maximum size the postings chunks will be in
         *  memory before they're written to disk.
         * @return whether the index creation was successful.
         */
        bool indexDocs(std::vector<Document> & documents, size_t chunkMBSize);

    private:

        /** contains term IDF info and where to find per-document posting information */
        Lexicon _lexicon;

        /** represents the large postings file on disk */
        Postings _postings;

        /** the tokenizer used to create the index */
        Tokenizer* _tokenizer;
};

#endif
