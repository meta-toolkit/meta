/**
 * @file filesystem.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <fstream>
#include <vector>

#ifndef META_HAS_EXPERIMENTAL_FILESYSTEM
#include <platformstl/filesystem/filesystem_traits.hpp>
#include <platformstl/filesystem/path.hpp>
#include <platformstl/filesystem/readdir_sequence.hpp>
#endif

#include "io/filesystem.h"
#include "io/mmap_file.h"
#include "util/printing.h"
#include "util/progress.h"

namespace meta
{
namespace filesystem
{

#if !defined META_HAS_EXPERIMENTAL_FILESYSTEM                                  \
    && !defined META_HAS_TR2_SYS_FILESYSTEM
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

bool file_exists(const std::string& filename)
{
    return traits::file_exists(filename.c_str());
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

    if (!traits::remove_directory(path.c_str()))
    {
        std::string error = "failed to recursively delete path ";
        error += path.c_str();
        throw filesystem_exception{error};
    }

    count += 1;
    return count;
}
}

std::uintmax_t remove_all(const std::string& path)
{
    return remove_all(path_type{path.c_str()});
}
#else // filesystem namespace exists, somewhere
#if META_HAS_EXPERIMENTAL_FILESYSTEM
namespace fs = std::experimental::filesystem;
#elif META_HAS_TR2_SYS_FILESYSTEM
using fs = std::tr2::sys::filesystem;
#endif

bool delete_file(const std::string& filename)
{
    return fs::remove(filename);
}

void rename_file(const std::string& old_name, const std::string& new_name)
{
    fs::rename(old_name, new_name);
}

bool make_directory(const std::string& dir_name)
{
    return fs::create_directory(dir_name);
}

bool file_exists(const std::string& filename)
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
    return fs::remove_all(path);
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
        uint64_t buf_size = 1024UL * 1024UL * 32UL; // 32 MB buffer
        uint64_t total_processed = 0;
        std::vector<char> buffer(buf_size);
        while (source_file)
        {
            source_file.read(buffer.data(), buf_size);
            auto processed = source_file.gcount();
            total_processed += processed;
            dest_file.write(buffer.data(), processed);
            prog(total_processed);
        }
        prog.end();
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

    printing::progress progress{" > Counting lines in file: ", file.size(), 500,
                                32 * 1024 * 1024};
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

    return num;
}
}
}
