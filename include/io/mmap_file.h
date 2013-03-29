/**
 * @file mmap_file.h
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
class MmapFile
{
    public:

        /**
         * Constructor.
         * @param path - creates a TextFile object from the given filename
         */
        MmapFile(std::string path);

        /**
         * Destructor; deallocates memory used to store this object, closing the text file.
         */
        ~MmapFile();

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
        MmapFile(const MmapFile & other) = delete;

        /** no copying */
        const MmapFile & operator=(const MmapFile & other) = delete;

    public:

        /**
         * Basic exception for MmapFile interactions.
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
