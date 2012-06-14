/**
 * @file compressed_file_writer.h
 */

#ifndef _COMPRESSED_FILE_WRITER_H_
#define _COMPRESSED_FILE_WRITER_H_

#include <string.h>
#include <cstdio>
#include <cmath>
#include <string>
#include <iostream>

using std::cerr;
using std::endl;
using std::string;

/**
 * Writes to a file of unsigned integers using gamma compression.
 */
class CompressedFileWriter
{
    public:

        /**
         * Constructor.
         * Opens a compressed file for writing or creates a new file if it doesn't exist.
         */
        CompressedFileWriter(const string & filename);

        /**
         * Destructor; closes the compressed file.
         */
        ~CompressedFileWriter();

        /**
         * Writes a value to the end of the compressed file.
         * @param value - the number to write
         */
        void write(unsigned int value);

    private:

        FILE* _outfile;
        unsigned int _charCursor;
        unsigned int _bitCursor;
        unsigned char* _buffer;
        unsigned int _bufferSize;

        /**
         * Writes a bit to the file and advances writeCursors.
         * @param bit
         */
        void writeBit(bool bit);

        /**
         * Writes the buffer to the file.
         */
        void writeBuffer() const;
};

#endif
