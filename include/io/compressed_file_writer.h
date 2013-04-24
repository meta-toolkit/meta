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
class compressed_file_writer
{
    public:

        /**
         * Constructor.
         * Opens a compressed file for writing or creates a new file if it doesn't exist.
         */
        compressed_file_writer(const std::string & filename);

        /**
         * Destructor; closes the compressed file.
         */
        ~compressed_file_writer();

        /**
         * Writes a value to the end of the compressed file.
         * @param value - the number to write
         */
        void write(unsigned int value);

    private:

        /** where to write the compressed data */
        FILE* _outfile;
        
        /** the current byte this reader is on */
        unsigned int _char_cursor;
        
        /** the current bit of the current byte this reader is on */
        unsigned int _bit_cursor;
        
        /** saved data that is not yet written to disk */
        unsigned char* _buffer;

        /** how large to make the internal writer buffer */
        unsigned int _buffer_size;

        /**
         * Writes a bit to the file and advances writeCursors.
         * @param bit
         */
        void write_bit(bool bit);

        /**
         * Writes the buffer to the file.
         */
        void write_buffer() const;

    public:

        /**
         * Basic exception for compressed_file_writer interactions.
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
