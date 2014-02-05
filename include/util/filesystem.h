/**
 * @file filesystem.h
 * Includes code shared by many classes.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _COMMON_FILESYSTEM_H_
#define _COMMON_FILESYSTEM_H_

#include <string>
#include <fstream>
#include <sys/stat.h>
#include "io/mmap_file.h"
#include "util/printing.h"
#include "util/progress.h"

namespace meta {
namespace filesystem {

/**
 * Deletes the given file.
 * @param filename
 */
inline void delete_file(const std::string & filename)
{
    remove(filename.c_str());
}

/**
 * Renames the given file.
 * @param old_name
 * @param new_name
 */
inline void rename_file(const std::string & old_name, const std::string & new_name)
{
    rename(old_name.c_str(), new_name.c_str());
}

/**
 * Attempts to create the directory
 * @return whether a new directory was created
 */
inline bool make_directory(const std::string & dir_name)
{
    return mkdir(dir_name.c_str(), 0755) == -1;
}

/**
 * @param filename
 * @return true if the file exists
 */
inline bool file_exists(const std::string & filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    if(f != nullptr)
    {
        fclose(f);
        return true;
    }
    return false;
}

/**
 * Copies a file source to file dest
 * @param source
 * @param dest
 * @return whether the copy was successful
 */
inline bool copy_file(const std::string & source, const std::string & dest)
{
    if(!file_exists(source))
        return false;

    std::ifstream source_file{source, std::ios::binary};
    std::ofstream dest_file{dest, std::ios::binary};
    dest_file << source_file.rdbuf();

    return true;
}

/**
 * Calculates a file's size in bytes with support for files over 4GB.
 * @param filename The path for the file
 * @return the number of bytes in the file
 */
inline uint64_t file_size(const std::string & filename)
{
    if(!file_exists(filename))
        return 0;

    struct stat64 st;
    stat64(filename.c_str(), &st);
    return st.st_size;
}

/**
 * @param filename
 * @param delimiter
 * @return the number of delimiter (default newline) characters in the
 * paramter
 */
inline uint64_t num_lines(const std::string & filename, char delimiter = '\n')
{
    io::mmap_file file{filename};
    uint64_t num = 0;

    printing::progress progress{" > Counting lines in file: ", file.size(), 500,
                                32 * 1024 * 1024};
    for(uint64_t idx = 0; idx < file.size(); ++idx)
    {
        progress(idx);
        if(file.start()[idx] == delimiter)
            ++num;
    }

    return num;
}

}
}

#endif
