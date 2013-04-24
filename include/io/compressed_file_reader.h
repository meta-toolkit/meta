/**
 * @file compressed_file_reader.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
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

namespace meta {
namespace io {

/**
 * Simply saves the current state of the reader.
 */
enum ReaderStatus { notDone, readerDone, userDone };

/**
 * Represents a file of unsigned integers compressed using gamma compression.
 */
class compressed_file_reader
{
    public:

        /**
         * Constructor.
         * Opens a compressed file for reading.
         */
        compressed_file_reader(const std::string & filename);

        /**
         * Destructor.
         * Closes the compressed file.
         */
        ~compressed_file_reader();

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
        void seek(unsigned int position, unsigned int bit_offset);

        /**
         * @return whether there is another number in the file
         */
        bool has_next() const;

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
        unsigned int _current_value;

        /** current byte in the compressed file */
        unsigned int _current_char;

        /** current bit inside the current byte */
        unsigned int _current_bit;

        /**
         * Sets _currentValue to the value of the next number.
         */
        void get_next();

        /**
         * Advances readCursors.
         * @return the next bit in the file
         */
        bool read_bit();

    public:

        /**
         * Basic exception for compressed_file_reader interactions.
         */
        class compressed_file_reader_exception: public std::exception
        {
            public:
                
                compressed_file_reader_exception(const std::string & error):
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
