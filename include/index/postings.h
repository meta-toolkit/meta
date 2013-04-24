/**
 * @file postings.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _POSTINGS_H_
#define _POSTINGS_H_

#include <memory>
#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

#include "index/document.h"
#include "index/lexicon.h"
#include "tokenizers/tokenizer.h"
#include "util/invertible_map.h"

namespace meta {
namespace index {

/**
 * This is the interface to the large postings file located on disk.
 */
class Postings
{
    public:

        /**
         * Constructor; sets this Postings object to look at the specified file.
         */
        Postings(const std::string & postingsFile);

        /**
         * @param termData - used to determine the location of the term information in the postings
         * @return a std::vector of documents that contain the term the parameters refer to
         */
        std::vector<PostingData> getDocs(const TermData & termData) const;

        /**
         * @param termData - used to determine the location of the term information in the postings
         * @return a std::vector of documents that contain the term the parameters refer to
         */
        std::vector<PostingData> getCompressedDocs(const TermData & termData) const;

        /**
         * Creates lists of term information sorted by term_id on disk.
         * The lexicon is NOT updated in this function.
         * @param documents - a list of documents to index
         * @param chunkMBSize - the maximum size the postings chunks will be in
         *  memory before they're written to disk.
         * @param tokenizer - how to tokenize the indexed documents
         * @return the number of chunks created. Since their name is standard, they can easily
         *  be located.
         */
        size_t createChunks(std::vector<document> & documents, size_t chunkMBSize,
                std::shared_ptr<tokenizers::tokenizer> tokenizer);

        /**
         * Creates the large postings file on disk out of many chunks.
         * @param numChunks - a list of filenames indicating the location of the chunks to combine
         * @param lexicon - the lexicon to update while indexing
         */
        void createPostingsFile(size_t numChunks, Lexicon & lexicon);

        /**
         * Creates a compressed postings file on disk out of many chunks.
         * @param numChunks - a list of filenames indicating the location of the chunks to combine
         * @param lexicon - the lexicon to update while indexing
         */
        void createCompressedPostingsFile(size_t numChunks, Lexicon & lexicon);

        /**
         * Saves the docid mapping to disk.
         * @param filename - the filename to save the mapping as
         */
        void savedoc_idMapping(const std::string & filename) const;

        /**
         * Saves document lengths for the given std::vector of documents.
         * @param documents - the documents to save lengths of
         * @param filename - the name for the doc length file
         */
        void saveDocLengths(const std::vector<document> & documents, const std::string & filename);

    private:

        /** Maintains the filename of the postings file */
        std::string _postingsFilename;
        
        //CompressedFileReader _reader;
        
        /** Keeps track of the doc_id -> filename mapping */
        util::invertible_map<doc_id, std::string> _docMap;

        /** Internal counter of docs, used to create doc_ids */
        doc_id _currentdoc_id;

        /**
         * @param pdata - list of PostingData for a term
         * @return the number of times a specific term has appeared in the corpus
         */
        unsigned int getTotalFreq(const std::vector<PostingData> & pdata) const;

        /**
         * Gets a line out of an uncompressed postings file.
         * This will be slow for large posting files and should
         *  only be used to check correctness.
         * @param lineNumber - which line number in the postings file to seek to
         * @return the contents of the specified line
         */
        std::string getLine(unsigned int lineNumber) const;

        /**
         * Writes a chunk to disk.
         * @param terms - the map of terms to write. It is cleared at the end of this function.
         * @param chunkNum - the number used for the filename
         */
        void writeChunk(std::map<term_id, std::vector<PostingData>> & terms, size_t chunkNum) const;

        /**
         * Keeps the doc_id -> path mapping, and returns a new doc_id
         *  if an unseen path is encountered.
         * @param path - the document path to check
         * @return the doc_id for the given path
         */
        doc_id getdoc_id(const std::string & path);
};

}
}

#endif
