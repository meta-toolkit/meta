/**
 * @file ram_index.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _RAM_INDEX_
#define _RAM_INDEX_

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include "tokenizers/tokenizer.h"
#include "index/index.h"

namespace meta {
namespace index {

/**
 * Represents an index that resides in memory and is created on the fly.
 */
class RAMIndex : public Index
{
    public:
        /**
         * Creates an Index located in memory.
         * @param indexDocs Untokenized Document objects to index
         * @param tokenizer How to tokenize the indexed files 
         */
        RAMIndex(const std::vector<Document> & indexDocs, std::shared_ptr<tokenizers::tokenizer> tokenizer);

        /**
         * Scores a document given a query.
         * @param document The doc to score
         * @param query The query to score against
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

        /** the tokenizer that was used to interpret the indexed documents */
        std::shared_ptr<tokenizers::tokenizer> _tokenizer;

        /** documents stored in this index */
        std::vector<Document> _documents;

        /** the IDF values of all the terms encountered in the document collection */
        std::shared_ptr<std::unordered_map<term_id, unsigned int>> _docFreqs;

        /** average number of terms in the documents in the collection */
        size_t _avgDocLength;

        /**
         * @return a filename minus the path
         */
        std::string shortFilename(const std::string & filename) const;
};

}
}

#endif
