/**
 * @file file_reader.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_FILE_READER_H_
#define _META_FILE_READER_H_

#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace meta {
namespace io {

/**
 * Reads binary objects from a file.
 */
class file_reader
{
    public:
        /**
         * Constructor; opens a file for reading.
         * @param filename The name of the file to read
         */
        file_reader(const std::string & filename);

        /**
         * Destructor; ensures the file is closed.
         */
        ~file_reader();

        /**
         * Reads an integral type from the file.
         */
        template <class T>
        void read(T & elem);

        /**
         * Reads a std::string type from the file.
         */
        void read(std::string & str);

        // disable copy-construction
        file_reader(const file_reader &) = delete;

        // disable assignment
        file_reader & operator=(const file_reader &) = delete;

    private:
        int _file_desc;
};

}
}

#include "io/file_reader.tcc"
#endif
