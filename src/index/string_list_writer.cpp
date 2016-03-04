/**
 * @file string_list_writer.cpp
 * @author Chase Geigle
 */

#include "meta/io/binary.h"
#include "meta/index/string_list_writer.h"

namespace meta
{
namespace index
{

string_list_writer::string_list_writer(const std::string& path, uint64_t size)
    : string_file_{path},
      write_pos_{0},
      index_{path + "_index", size}
{
    // nothing
}

string_list_writer::string_list_writer(string_list_writer&& other)
    : string_file_{std::move(other.string_file_)},
      write_pos_{std::move(other.write_pos_)},
      index_{std::move(other.index_)}
{
    // nothing
}

string_list_writer& string_list_writer::operator=(string_list_writer&& other)
{
    if (this != &other)
    {
        string_file_ = std::move(other.string_file_);
        write_pos_ = std::move(other.write_pos_);
        index_ = std::move(other.index_);
    }
    return *this;
}

void string_list_writer::insert(uint64_t idx, const std::string& elem)
{
    std::lock_guard<std::mutex> lock{mutex_};
    index_[idx] = write_pos_;
    io::write_binary(string_file_, elem);
    write_pos_ += elem.length() + 1;
}
}
}
