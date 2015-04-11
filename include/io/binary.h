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
void write_binary(std::ostream& out, const T& elem)
{
    out.write(reinterpret_cast<const char*>(&elem), sizeof(T));
}

/**
 * Writes a std::string object in binary format to a stream.
 * @param out The stream to write to
 * @param str the string to write
 */
inline void write_binary(std::ostream& out, const std::string& str)
{
    out.write(str.c_str(), str.size() + 1);
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

/**
 * Writes an integral type in a packed representation. The first byte is a
 * flag byte used to indicate two things: the first bit indicates the sign
 * of the number, and then the lowest four bits indicates the length (in
 * bytes) of the unsigned number that follows.
 *
 * @see http://dlib.net/dlib/serialize.h.html
 * @param out The stream to write to
 * @param elem The integral type to write in packed format
 */
template <class T>
void write_packed_binary(std::ostream& out, T elem)
{
    static_assert(std::is_integral<T>::value,
                  "packed binary requires integers");

    std::array<uint8_t, sizeof(T) + 1> buffer;
    if (elem < 0)
    {
        elem *= -1;
        buffer[0] = 0x80;
    }
    else
    {
        buffer[0] = 0;
    }

    uint8_t idx = 1;
    for (; idx <= sizeof(T) && elem > 0; ++idx)
    {
        buffer[idx] = static_cast<uint8_t>(elem & 0xFF);
        elem >>= 8;
    }
    buffer[0] |= (idx - 1);
    out.write(reinterpret_cast<char*>(&buffer[0]), idx);
}

/**
 * Writes a double in a packed integer binary representation. This splits
 * the double into its mantissa and exponent such that
 * mantissa * std::pow(2.0, exponent) == elem. The mantissa and exponent
 * are integers are are written using the integer packed format.
 *
 * @see
 *http://stackoverflow.com/questions/5672960/how-can-i-extract-the-mantissa-of-a-double
 * @see http://dlib.net/dlib/float_details.h.html
 * @param out The stream to write to
 * @param elem The double to write in packed format
 */
inline void write_packed_binary(std::ostream& out, double elem)
{
    int exp;
    auto digits = std::numeric_limits<double>::digits;
    auto mantissa
        = static_cast<int64_t>(std::frexp(elem, &exp) * (1ul << digits));
    int16_t exponent = exp - digits;

    // see dlib link above; tries to shrink mantissa for more efficient
    // serialization
    for (uint8_t i = 0; i < sizeof(mantissa) && (mantissa & 0xFF) == 0; ++i)
    {
        mantissa >>= 8;
        exponent += 8;
    }

    write_packed_binary(out, mantissa);
    write_packed_binary(out, exponent);
}

/**
 * Reads an integer from its packed binary representation.
 * @param in The stream to read from
 * @param elem The element to write into
 */
template <class InputStream, class T>
void read_packed_binary(InputStream& in, T& elem)
{
    static_assert(std::is_integral<T>::value,
                  "packed binary requires integers");

    auto flag_byte = static_cast<uint8_t>(in.get());
    auto size = flag_byte & 0x0F;

    elem = 0;
    for (uint8_t idx = 0; idx < size; ++idx)
    {
        auto byte = static_cast<uint64_t>(in.get());
        byte <<= 8 * idx;
        elem |= byte;
    }

    if (std::is_signed<T>::value && (flag_byte & 0x80) > 0)
    {
        elem *= -1;
    }
}

/**
 * Reads a double from its packed binary representation.
 * @param in The stream to read from
 * @param elem The element to write into
 */
template <class InputStream>
void read_packed_binary(InputStream& in, double& elem)
{
    int64_t mantissa;
    int16_t exponent;
    read_packed_binary(in, mantissa);
    read_packed_binary(in, exponent);
    elem = mantissa * std::pow(2.0, exponent);
}
}
}
#endif
