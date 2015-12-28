/**
 * @file filesystem.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FILESYSTEM_H_
#define META_FILESYSTEM_H_

#include <cstdint>
#include <stdexcept>
#include <string>

#ifdef META_HAS_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#endif


namespace meta
{
namespace filesystem
{

#ifndef META_HAS_EXPERIMENTAL_FILESYSTEM
class filesystem_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
#else
using filesystem_exception = std::experimental::filesystem::filesystem_error;
#endif

/**
 * Deletes the given file.
 * @param filename The file to delete (path)
 */
bool delete_file(const std::string& filename);

/**
 * Renames the given file.
 * @param old_name The old filename
 * @param new_name The new filename
 */
void rename_file(const std::string& old_name, const std::string& new_name);

/**
 * Attempts to create the directory
 * @param dir_name The name of the new directory
 * @return whether a new directory was created
 */
bool make_directory(const std::string& dir_name);

/**
 * @param filename The file to check
 * @return true if the file exists
 */
bool file_exists(const std::string& filename);

/**
 * Calculates a file's size in bytes with support for files over 4GB.
 * @param filename The path for the file
 * @return the number of bytes in the file
 */
uint64_t file_size(const std::string& filename);

/**
 * Copies a file source to file dest.
 * @param source The source file
 * @param dest The destination file
 * @return whether the copy was successful
 */
bool copy_file(const std::string& source, const std::string& dest);

/**
 * @param in_name The filename to read
 * @return string content from the given file
 */
std::string file_text(const std::string& in_name);

/**
 * @param filename The file to count lines in
 * @param delimiter How to denote lines
 * @return the number of delimiter (default newline) characters in the
 * paramter
 */
uint64_t num_lines(const std::string& filename, char delimiter = '\n');

/**
 * Removes the contents of path (if it is a directory) and the contents of
 * all of its subdirectories, recursively, then deletes path itself.
 *
 * @param path The path to delete
 * @return the number of files and directories that were deleted
 */
std::uintmax_t remove_all(const std::string& path);
}
}
#endif
