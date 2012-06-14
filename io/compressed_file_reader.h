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

using std::cerr;
using std::endl;
using std::string;

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
        CompressedFileReader(const string & filename);

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

        unsigned char* _start;
        int _fileDescriptor;
        unsigned int _size;
        int _status;

        unsigned int _currentValue;
        unsigned int _currentChar;
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
