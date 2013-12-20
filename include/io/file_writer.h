/**
 * @file file_writer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_FILE_WRITER_H_
#define _META_FILE_WRITER_H_

#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace meta {
namespace io {

/**
 * This class serves as a fast alternative to file stream operators.
 */
class file_writer
{
    public:
        /**
         * Constructor; opens a file for writing.
         * @param filename The name of the file to create
         */
        file_writer(const std::string & filename);

        /**
         * Destructor; ensures the file is closed.
         */
        ~file_writer();

        /**
         * Writes an integral type to the file.
         */
        template <class T>
        void write(const T & elem);

        /**
         * Writes a string object to the file.
         */
        void write(const std::string & str);

        // disable copy-construction
        file_writer(const file_writer &) = delete;

        // disable assignment
        file_writer & operator=(const file_writer &) = delete;

    private:
        int _file_desc;

    public:
        /**
         * Basic exception for file_writer interactions.
         */
        class file_writer_exception: public std::exception
        {
            public:
                
                file_writer_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what() const throw()
                {
                    return _error.c_str();
                }
           
            private:
           
                std::string _error;
        };
};

}
}

#include "io/file_writer.tcc"
#endif
