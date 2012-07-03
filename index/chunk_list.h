/**
 * @file chunk_list.h
 */

#ifndef _CHUNK_LIST_H_
#define _CHUNK_LIST_H_

#include <string>

using std::string;

/**
 * Represents a collection of chunks that are waiting to be merged into
 *  a postings file.
 */
class ChunkList
{
    public:

        /**
         * Constructor.
         * @param numChunks - this shows how many chunks have been
         *  generated for this index. It also allows the ChunkList
         *  to access the correct files.
         */
        ChunkList(size_t numChunks);

        /**
         * @return whether there is another string to write to the postings file
         */
        bool hasNext() const;

        /**
         * @return the next string to write to the postings file
         */
        string next();

    private:

        size_t _numChunks;
};

#endif
