/**
 * @file compressed_file_writer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _COMPRESSED_FILE_WRITER_H_
#define _COMPRESSED_FILE_WRITER_H_

#include <string.h>
#include <cstdio>
#include <cmath>
#include <string>

namespace meta {
namespace io {

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
        CompressedFileWriter(const std::string & filename);

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

        /** where to write the compressed data */
        FILE* _outfile;
        
        /** the current byte this reader is on */
        unsigned int _charCursor;
        
        /** the current bit of the current byte this reader is on */
        unsigned int _bitCursor;
        
        /** saved data that is not yet written to disk */
        unsigned char* _buffer;

        /** how large to make the internal writer buffer */
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

    public:

        /**
         * Basic exception for CompressedFileWriter interactions.
         */
        class compressed_file_writer_exception: public std::exception
        {
            public:
                
                compressed_file_writer_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }
           
            private:
           
                std::string _error;
        };

};

}
}

#endif
