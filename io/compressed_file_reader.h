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

#include <iostream>
#include <string>

/**
 * Simply saves the current state of the reader.
 */
enum ReaderStatus { notDone, readerDone, userDone };

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
        CompressedFileReader(const std::string & filename);

        /**
         * Destructor.
         * Closes the compressed file.
         */
        ~CompressedFileReader();

        /**
         * Sets the cursor back to the beginning of the file.
         */
        void reset();

        /**
         * Sets the cursor to the specified position in the file.
         * It is up to the user to specify a valid position.
         * @param position - where to set the cursor in terms of bytes
         * @param bitOffset - bit offset from current byte position [0..7]
         */
        void seek(unsigned int position, unsigned int bitOffset);

        /**
         * @return whether there is another number in the file
         */
        bool hasNext() const;

        /**
         * @return the next compressed number
         */
        unsigned int next();

    private:

        /** pointer to the beginning of the compressed file (which will be in
         * memory most of the time) */
        unsigned char* _start;

        /** file descriptor for the memory map where this compressed file is */
        int _fileDescriptor;

        /** the number of bytes in this compressed file */
        unsigned int _size;

        /** reading/writing status */
        int _status;

        /** current numeric value that was read */
        unsigned int _currentValue;

        /** current byte in the compressed file */
        unsigned int _currentChar;

        /** current bit inside the current byte */
        unsigned int _currentBit;

        /**
         * Sets _currentValue to the value of the next number.
         */
        void getNext();

        /**
         * Advances readCursors.
         * @return the next bit in the file
         */
        bool readBit();
};

#endif
