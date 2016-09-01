/**
 * @file binary.h
 * @author Chase Geigle
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_IO_BINARY_H_
#define META_IO_BINARY_H_

#include <array>
#include <cmath>
#include <istream>
#include <limits>
#include <ostream>

#include "meta/config.h"

namespace meta
{
namespace io
{

/**
 * Writes an object in binary format to a stream.
 * @param out The stream to write to
 * @param elem The element to write
 */
template <class T>
uint64_t write_binary(std::ostream& out, const T& elem)
{
    out.write(reinterpret_cast<const char*>(&elem), sizeof(T));
    return sizeof(T);
}

/**
 * Writes a std::string object in binary format to a stream.
 * @param out The stream to write to
 * @param str the string to write
 */
inline uint64_t write_binary(std::ostream& out, const std::string& str)
{
    out.write(str.c_str(), static_cast<std::streamsize>(str.size() + 1));
    return str.size() + 1;
}

/**
 * Reads an object in binary from a stream.
 * @param in The stream to read from
 * @param elem The element to read
 */
template <class T>
void read_binary(std::istream& in, T& elem)
{
    in.read(reinterpret_cast<char*>(&elem), sizeof(T));
}

/**
 * Reads a string in binary from a stream.
 * @param in The stream to read from
 * @param str The string to read
 */
inline void read_binary(std::istream& in, std::string& str)
{
    std::getline(in, str, '\0');
}
}
}
#endif
