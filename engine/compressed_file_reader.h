/**
 * @file compressed_file_reader.h
 */

#ifndef _COMPRESSED_FILE_READER_H_
#define _COMPRESSED_FILE_READER_H_

#include <cmath>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

using std::string;

/**
 * Represents a file of unsigned integers compressed using gamma compression.
 */
class CompressedFileReader
{
    public:

        /**
         * Constructor.
         * Opens a compressed file for reading.
         */
        CompressedFileReader(const string & filename);

        /**
         * Sets the cursor back to the beginning of the file.
         */
        void reset();

        /**
         * Sets the cursor to the specified position in the file.
         * It is up to the user to specify a valid position.
         * @param position - where to set the cursor
         */
        void seek(unsigned int position);

        /**
         * @return whether there is another number in the file
         */
        bool hasNext() const;

        /**
         * @return the next compressed number
         */
        unsigned int next();

    private:

        unsigned int readCharCursor;
        unsigned int readBitCursor;

        /**
         * Advances readCursors.
         * @return the next bit in the file
         */
        void readBit();
};

#endif
