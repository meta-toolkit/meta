/**
 * @file compressed_file_writer.h
 * @author Sean Massung
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
#include "util/invertible_map.h"

namespace meta {
namespace io {

/**
 * Writes to a file of unsigned integers using gamma compression.
 */
class compressed_file_writer
{
    public:

        /**
         * Constructor; Opens a compressed file for writing or creates a new
         * file if it doesn't exist.
         * @param filename The path to the compressed file
         * @param mapping
         */
        compressed_file_writer(const std::string & filename,
                std::function<uint64_t(uint64_t)> mapping);

        /**
         * Destructor; closes the compressed file.
         */
        ~compressed_file_writer();

        /**
         * @return the character index and bit index of the current location in
         * the compressed file
         */
        uint64_t bit_location() const;

        /**
         * Writes a value to the end of the compressed file.
         * @param value The number to write
         */
        void write(uint64_t value);

        /**
         * Writes a binary string to the file.
         */
        void write(const std::string & str);

        /**
         * Closes this compressed file.
         */
        void close();

    private:
        /**
         * Writes a bit to the file and advances writeCursors.
         * @param bit
         */
        void write_bit(bool bit);

        /**
         * Writes the buffer to the file.
         */
        void write_buffer() const;

        /** where to write the compressed data */
        FILE* _outfile;

        /** the current byte this reader is on */
        uint64_t _char_cursor;

        /** the current bit of the current byte this reader is on */
        uint64_t _bit_cursor;

        /** how large to make the internal writer buffer */
        uint64_t _buffer_size;

        /** saved data that is not yet written to disk */
        unsigned char* _buffer;

        /** the mapping to use (actual -> compressed id) */
        std::function<uint64_t(uint64_t)> _mapping;

        /** the number of total bits that have been written (for seeking )*/
        uint64_t _bit_location;

        /** ensures the file isn't closed more than once */
        bool _closed;

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
