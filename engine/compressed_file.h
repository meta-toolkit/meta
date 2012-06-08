/**
 * @file compressed_file.h
 */

#ifndef _COMPRESSED_FILE_H_
#define _COMPRESSED_FILE_H_

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
class CompressedFile
{
    public:

        /**
         * Constructor.
         * Opens a compressed file for reading and writing or creates a new file if it doesn't exist.
         */
        CompressedFile(const string & filename);

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

        /**
         * Writes a value to the end of the compressed file.
         * @param value - the number to write
         */
        void write(unsigned int value);

    private:

        unsigned int readCharCursor;
        unsigned int readBitCursor;

        // this is going to be gross
        // will have to stop in the middle of chars
        //  to read and write

        // how to store the last value? it will be padded with zeros or something??

        unsigned int writeCharCursor;
        unsigned int writeBitCursor;

        /**
         * Writes a bit to the file and advances writeCursors.
         * @param bit
         */
        void writeBit(bool bit);

        /**
         * Advances readCursors.
         * @return the next bit in the file
         */
        void readBit();
};

#endif
