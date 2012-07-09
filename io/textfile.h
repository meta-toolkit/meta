/**
 * @file textfile.h
 * Originally part of CS 296-25 Honors project Spring 2011.
 * Allows programming with (hopefully) fewer I/O bottlenecks
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
 */
class TextFile
{
    public:

        /**
         * Constructor.
         * @param title - creates a TextFile object from the given filename
         */
        TextFile(std::string title);

        /**
         * Opens the current file.
         * @return a pointer to the beginning of the text file; NULL if unsuccessful
         */
        char* opentext();

        /**
         * @return the length of the file (in number of characters)
         */
        unsigned int get_size() const;

        /**
         * @return the title of the text file (the parameter given to the contructor)
         */
        std::string get_title() const;

        /**
         * Closes the text file.
         * @return whether closing the file was successful
         */
        bool closetext();

    private:

        std::string _title;
        char* start;
        unsigned int size;
        int file_descriptor;
};

#endif
