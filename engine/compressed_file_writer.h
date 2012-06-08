/**
 * @file compressed_file_writer.h
 */

#ifndef _COMPRESSED_FILE_WRITER_H_
#define _COMPRESSED_FILE_WRITER_H_

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
         * Writes a value to the end of the compressed file.
         * @param value - the number to write
         */
        void write(unsigned int value);

    private:

        unsigned int writeCharCursor;
        unsigned int writeBitCursor;

        /**
         * Writes a bit to the file and advances writeCursors.
         * @param bit
         */
        void writeBit(bool bit);
};

#endif
