/**
 * @file mmap_file.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _MMAP_FILE_H_
#define _MMAP_FILE_H_

#include <stdexcept>
#include <string>

namespace meta {
namespace io {

/**
 * Memory maps a text file for better I/O performance and allows you to read it.
 */
class mmap_file
{
    public:

        /**
         * Constructor.
         * @param path Path to the text file to open
         */
        mmap_file(const std::string & path);

        /**
         * Move constructor.
         */
        mmap_file(mmap_file&&);

        /**
         * Move assignment operator.
         */
        mmap_file& operator=(mmap_file&&);

        /**
         * Destructor; deallocates memory used to store this object, closing the
         * text file.
         */
        ~mmap_file();

        /**
         * @return a pointer to the beginning of the text file; nullptr if
         * unsuccessful
         */
        char* start() const;

        /**
         * @return the length of the file in bytes
         */
        uint64_t size() const;

        /**
         * @return the title of the text file (the parameter given to the
         * contructor)
         */
        std::string path() const;

    private:

        /** filename of the text file */
        std::string _path;

        /** pointer to the beginning of the text file */
        char* _start;

        /** size of the current text file */
        uint64_t _size;

        /** file descriptor for the open text file */
        int _file_descriptor;

        /** no copying */
        mmap_file(const mmap_file & other) = delete;

        /** no copying */
        const mmap_file & operator=(const mmap_file & other) = delete;

    public:

        /**
         * Basic exception for mmap_file interactions.
         */
        class mmap_file_exception: public std::runtime_error
        {
            public:
                using std::runtime_error::runtime_error;
        };
};

}
}

#endif
