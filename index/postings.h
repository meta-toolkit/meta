/**
 * @file postings.h
 */

#ifndef _POSTINGS_H_
#define _POSTINGS_H_

#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include "tokenizers/tokenizer.h"
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "lexicon.h"

using std::map;
using std::istringstream;
using std::ifstream;
using std::string;
using std::vector;
using std::cerr;
using std::endl;

/**
 * Represents one term's document info.
 */
struct PostingData
{
    /** The numeric id value assigned to this document */
    DocID docID;

    /** The number of times a term appeared in this document */
    unsigned int freq;

    /** Parameters constructor */
    PostingData(DocID pdocID, unsigned int pfreq):
        docID(pdocID), freq(pfreq){ /* nothing */ }

    /** No params contructor */
    PostingData():
        docID(0), freq(0){ /* nothing */ }
};

/**
 * This is the interface to the large postings file located on disk.
 */
class Postings
{
    public:

        /**
         * Constructor; sets this Postings object to look at the specified file.
         */
        Postings(const string & postingsFile);

        /**
         * @param termData - used to determine the location of the term information in the postings
         * @return a vector of documents that contain the term the parameters refer to
         */
        vector<PostingData> getDocs(const TermData & termData) const;

        /**
         * @param termData - used to determine the location of the term information in the postings
         * @return a vector of documents that contain the term the parameters refer to
         */
        vector<PostingData> getCompressedDocs(const TermData & termData) const;

        /**
         * Creates lists of term information sorted by TermID on disk.
         * The lexicon is NOT updated in this function.
         * @param documents - a list of documents to index
         * @param chunkMBSize - the maximum size the postings chunks will be in
         *  memory before they're written to disk.
         * @param tokenizer - how to tokenize the indexed documents
         * @return the number of chunks created. Since their name is standard, they can easily
         *  be located.
         */
        size_t createChunks(vector<Document> & documents, size_t chunkMBSize, Tokenizer* tokenizer) const;

        /**
         * Creates the large postings file on disk out of many chunks.
         * @param chunks - a list of filenames indicating the location of the chunks to combine
         */
        void createPostingsFile(size_t numChunks);

        /**
         * Creates a compressed postings file on disk out of many chunks.
         * @param chunks - a list of filenames indicating the location of the chunks to combine
         */
        void createCompressedPostingsFile(size_t numChunks);

    private:

        string _postingsFilename;
        CompressedFileReader _reader;

        /**
         * Gets a line out of an uncompressed postings file.
         * This will be slow for large posting files and should
         *  only be used to check correctness.
         * @param lineNumber - which line number in the postings file to seek to
         * @return the contents of the specified line
         */
        string getLine(unsigned int lineNumber) const;

        /**
         * Writes a chunk to disk.
         * @param terms - the map of terms to write. It is cleared at the end of this function.
         * @param chunkNum - the number used for the filename
         */
        void writeChunk(map<TermID, vector<PostingData>> & terms, size_t chunkNum) const;
};

#endif
