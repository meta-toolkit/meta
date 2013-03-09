/**
 * @file textfile.h
 * Allows programming with (hopefully) fewer I/O bottlenecks.
 */

#ifndef _TEXTFILE_H_
#define _TEXTFILE_H_

#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Memory maps a text file for better I/O performance and allows you to read it.
 * However, if the file is very small, it will simply be stored in memory.
 */
class TextFile
{
    public:

        /**
         * Constructor.
         * @param path - creates a TextFile object from the given filename
         */
        TextFile(std::string path);

        /**
         * Destructor; deallocates memory used to store this object, closing the text file.
         */
        ~TextFile();

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

        /** minimum size requirement for mmap'ing files */
        static const unsigned int _min_mmap_size = 4096;

        /** filename of the text file */
        std::string _path;

        /** pointer to the beginning of the text file */
        char* _start;

        /** size of the current text file */
        unsigned int _size;

        /** file descriptor for the open text file */
        int _file_descriptor;

        /**
         * Loads a small file onto the heap in order to save virtual memory space. It's supposed to
         * be more efficient, and doesn't waste space since mmap takes up one page minimum per file.
         */
        void load_file();

        /**
         * Memory-maps the file.
         */
        void open_mmap();

        /** no copying */
        TextFile(const TextFile & other) = delete;

        /** no copying */
        const TextFile & operator=(const TextFile & other) = delete;
};

#endif
