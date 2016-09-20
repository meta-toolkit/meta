/**
 * @file filesystem.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <fstream>
#include <vector>

#include "meta/io/filesystem.h"
#include "meta/io/mmap_file.h"
#include "meta/util/printing.h"
#include "meta/util/progress.h"

#if META_HAS_EXPERIMENTAL_FILESYSTEM == 0
// reordering these includes screws stuff up, so leave whitespace between
// them so clang-format won't reorder
#include <platformstl/filesystem/filesystem_traits.hpp>

#include <platformstl/filesystem/path.hpp>

#include <platformstl/filesystem/readdir_sequence.hpp>

#include <platformstl/filesystem/directory_functions.hpp>
#else

#ifdef _WIN32
// chrono and thread for a sleep_for hack in remove_all below
#include <chrono>
#include <thread>
#endif

#include <experimental/filesystem>
#endif

namespace meta
{
namespace filesystem
{

#if META_HAS_EXPERIMENTAL_FILESYSTEM == 0
namespace
{
using traits = platformstl::filesystem_traits<char>;
}

bool delete_file(const std::string& filename)
{
    return traits::unlink_file(filename.c_str());
}

void rename_file(const std::string& old_name, const std::string& new_name)
{
    if (!traits::rename_file(old_name.c_str(), new_name.c_str()))
        throw filesystem_exception{"failed to rename file " + old_name};
}

bool make_directory(const std::string& dir_name)
{
    return traits::create_directory(dir_name.c_str());
}

bool make_directories(const std::string& path)
{
    return stlsoft::platformstl_project::create_directory_recurse(path);
}

bool file_exists(const std::string& filename)
{
    return traits::file_exists(filename.c_str());
}

bool exists(const std::string& filename)
{
    return file_exists(filename);
}

uint64_t file_size(const std::string& filename)
{
    if (!file_exists(filename))
        return 0;

    traits::stat_data_type st;
    if (!traits::stat(filename.c_str(), &st))
        return 0;

    return traits::get_file_size(&st);
}

namespace
{
using path_type = platformstl::basic_path<char>;
std::uintmax_t remove_all(const path_type& path)
{
    if (!traits::file_exists(path.c_str()))
        return 0;

    traits::stat_data_type st;
    if (!traits::stat(path.c_str(), &st))
        return 0;

    if (traits::is_file(&st) || traits::is_link(&st))
    {
        if (traits::unlink_file(path.c_str()))
            return 1;
        std::string error = "failled to remove leaf file ";
        error += path.c_str();
        throw filesystem_exception{error};
    }

    std::uintmax_t count = 0;
    for (const auto& p : platformstl::readdir_sequence{path})
    {
        path_type nextpath{path};
        nextpath.push_sep();
        nextpath.push(p);
        count += remove_all(nextpath);
    }

#ifdef _WIN32
    // On Windows, ::DeleteFile() only "marks a file for deletion on close"
    // and therefore "the file deletion does not occur until the last
    // handle to the file is closed". This means that, in some cases, the
    // call to remove_directory below can fail if another process (like the
    // Windows search indexer or a virus scanner) happens to still have one
    // of the files in this directory that we deleted above open when we
    // attempt to remove it.
    //
    // As a workaround, we'll attempt to remove a directory a maximum of
    // three times, sleeping for 100ms more between each successive try. If
    // removing the directory fails three times in a row, we will throw an
    // exception and bail out. I really wish there were a better workaround
    // than this, but I can't come up with anything and we want
    // remove_all() to have the same (sane) semantics as it would have on
    // Unix platforms.
    std::chrono::milliseconds delay(0);
    for (int i = 0; i < 3; ++i)
    {
        if (traits::remove_directory(path.c_str()))
        {
            count += 1;
            return count;
        }
        delay += std::chrono::milliseconds(100);
        std::this_thread::sleep_for(delay);
    }

    // failed too many times
    std::string error = "failed to recursively delete path ";
    error += path.c_str();
    throw filesystem_exception{error};
#else
    if (!traits::remove_directory(path.c_str()))
    {
        std::string error = "failed to recursively delete path ";
        error += path.c_str();
        throw filesystem_exception{error};
    }

    count += 1;
    return count;
#endif
}
}

std::uintmax_t remove_all(const std::string& path)
{
    return remove_all(path_type{path.c_str()});
}
#else
namespace fs = std::experimental::filesystem;

bool delete_file(const std::string& filename)
{
    return fs::exists(filename) && fs::remove(filename);
}

void rename_file(const std::string& old_name, const std::string& new_name)
{
    fs::rename(old_name, new_name);
}

bool make_directory(const std::string& dir_name)
{
    return fs::create_directory(dir_name);
}

bool make_directories(const std::string& path)
{
    return fs::create_directories(path);
}

bool file_exists(const std::string& filename)
{
    return fs::exists(filename);
}

bool exists(const std::string& filename)
{
    return fs::exists(filename);
}

uint64_t file_size(const std::string& filename)
{
    if (!file_exists(filename))
        return 0;
    return fs::file_size(filename);
}

std::uintmax_t remove_all(const std::string& path)
{
    if (!fs::exists(path))
        return 0;
#if META_HAS_EXPERIMENTAL_FILESYSTEM_REMOVE_ALL
    return fs::remove_all(path);
#else
    // fs::remove_all doesn't properly recurse on directories, so we get
    // to...
    std::uintmax_t count = 1;
    if (fs::is_directory(path))
    {
        for (fs::directory_iterator d{path}, end; d != end; ++d)
            count += meta::filesystem::remove_all(d->path());
    }
    fs::remove(path);
    return count;
#endif
}
#endif

bool copy_file(const std::string& source, const std::string& dest)
{
    if (!file_exists(source))
        return false;

    // if file is larger than 128 MB, show copy progress
    auto size = file_size(source);
    uint64_t max_size = 1024UL * 1024UL * 1024UL * 128UL;
    if (size > max_size)
    {
        printing::progress prog{"Copying file ", size};
        std::ifstream source_file{source, std::ios::binary};
        std::ofstream dest_file{dest, std::ios::binary};
        std::streamsize buf_size = 1024UL * 1024UL * 32UL; // 32 MB buffer
        uint64_t total_processed = 0;
        std::vector<char> buffer(static_cast<std::size_t>(buf_size));
        while (source_file)
        {
            source_file.read(buffer.data(), buf_size);
            auto processed = source_file.gcount();
            total_processed += static_cast<std::size_t>(processed);
            dest_file.write(buffer.data(), processed);
            prog(total_processed);
        }
    }
    // otherwise, copy the file normally
    else
    {
        std::ifstream source_file{source, std::ios::binary};
        std::ofstream dest_file{dest, std::ios::binary};
        dest_file << source_file.rdbuf();
    }

    return true;
}

std::string file_text(const std::string& in_name)
{
    std::ifstream infile{in_name};
    std::ostringstream buf;
    buf << infile.rdbuf();
    return buf.str();
}

uint64_t num_lines(const std::string& filename, char delimiter /*= '\n'*/)
{
    io::mmap_file file{filename};
    uint64_t num = 0;

    {
        printing::progress progress{" > Counting lines in file: ", file.size()};
        for (uint64_t idx = 0; idx < file.size(); ++idx)
        {
            progress(idx);
            if (file[idx] == delimiter)
                ++num;
        }

        // this fixes a potential off-by-one if the last line in the file
        // doesn't end with the delimiter
        if (file[file.size() - 1] != delimiter)
            ++num;
    }

    return num;
}
}
}
