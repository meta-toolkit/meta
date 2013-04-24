/**
 * @file mmap_file.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _MMAP_FILE_H_
#define _MMAP_FILE_H_

#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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
         * @param path - creates a TextFile object from the given filename
         */
        mmap_file(std::string path);

        /**
         * Destructor; deallocates memory used to store this object, closing the text file.
         */
        ~mmap_file();

        /**
         * @return a pointer to the beginning of the text file; nullptr if unsuccessful
         */
        char* start() const;

        /**
         * @return the length of the file in bytes
         */
        unsigned int size() const;

        /**
         * @return the title of the text file (the parameter given to the contructor)
         */
        std::string path() const;

    private:

        /** filename of the text file */
        std::string _path;

        /** pointer to the beginning of the text file */
        char* _start;

        /** size of the current text file */
        unsigned int _size;

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
        class mmap_file_exception: public std::exception
        {
            public:
                
                mmap_file_exception(const std::string & error):
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
