/**
 * @file packed.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_IO_PACKED_H_
#define META_IO_PACKED_H_

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include "util/identifiers.h"
#include "util/string_view.h"

namespace meta
{
namespace io
{
namespace packed
{

/**
 * Writes an unsigned integer in a packed representation. Each byte has
 * seven bits of data, with the MSB reserved for a flag indicating whether
 * to continue reading bytes.
 *
 * @param stream The stream to write to
 * @param value The value to write
 * @return the number of bytes used to write out the value
 */
template <class OutputStream, class T>
typename std::enable_if<!std::is_floating_point<T>::value
                            && std::is_unsigned<T>::value
                            && !std::is_same<T, bool>::value,
                        uint64_t>::type
write(OutputStream& stream, T value)
{
    uint64_t size = 0;
    while (value > 127)
    {
        ++size;
        stream.put(static_cast<char>((value & 127) | 128));
        value >>= 7;
    }
    stream.put(static_cast<char>(value));
    return size + 1;
}

/**
 * Writes a boolean in a packed representation. We can't really do much
 * better than a single byte, so we just store a 0 if the value was true
 * and a 1 if the value was false.
 */
template <class OutputStream, class T>
typename std::enable_if<std::is_same<T, bool>::value, uint64_t>::type
write(OutputStream& stream, T value)
{
    uint8_t val = value ? 1 : 0;
    return write(stream, val);
}

/**
 * Writes a signed integer in a packed representation. This uses the same
 * representation as for unsigned integers by first converting the signed
 * integer into an unsigned one using zig-zag encoding.
 *
 * @see https://developers.google.com/protocol-buffers/docs/encoding#types
 *
 * @param stream The stream to write to
 * @param value The value to write
 * @return the number of bytes used to write out the value
 */
template <class OutputStream, class T>
typename std::enable_if<!std::is_floating_point<T>::value
                            && std::is_signed<T>::value,
                        uint64_t>::type
write(OutputStream& stream, T value)
{
    using usigned_type = typename std::make_unsigned<T>::type;
    auto elem = static_cast<usigned_type>((value << 1)
                                          ^ (value >> (sizeof(T) * 8 - 1)));
    return write(stream, elem);
}

/**
 * Writes a double in a packed representation. This splits
 * the double into its mantissa and exponent such that
 * mantissa * std::pow(2.0, exponent) == elem. The mantissa and exponent
 * are integers are are written using the integer packed format.
 *
 * @see
 *http://stackoverflow.com/questions/5672960/how-can-i-extract-the-mantissa-of-a-double
 * @see http://dlib.net/dlib/float_details.h.html
 *
 * @param stream The stream to write to
 * @param value The value to write
 * @return the number of bytes used to write out the value
 */
template <class OutputStream, class T>
typename std::enable_if<std::is_floating_point<T>::value, uint64_t>::type
write(OutputStream& stream, T value)
{
    int exp;
    auto digits = std::numeric_limits<T>::digits;
    auto mantissa = static_cast<int64_t>(std::frexp(value, &exp)
                                         * (uint64_t{1} << digits));
    int64_t exponent = exp - digits;

    // see dlib link above; tries to shrink mantissa for more efficient
    // serialization
    for (uint8_t i = 0; i < sizeof(mantissa) && (mantissa & 0xFF) == 0; ++i)
    {
        mantissa >>= 8;
        exponent += 8;
    }

    auto bytes = write(stream, mantissa);
    bytes += write(stream, exponent);
    return bytes;
}

/**
 * Writes a string in a packed representation. At the moment, the most
 * efficient thing I can think to do here is just write it out as a
 * standard C-string.
 *
 * @param stream The stream to write to
 * @param value The value to write
 * @return the number of bytes used to write out the value
 */
template <class OutputStream>
uint64_t write(OutputStream& stream, util::string_view value)
{
    for (const auto& c : value)
    {
        stream.put(c);
    }
    stream.put('\0');
    return value.size() + 1;
}

/**
 * Writes an enumeration type in a packed representation. This determines
 * the underlying type and serializes that.
 *
 * @param stream The stream to write to
 * @param value The value to write
 * @return the number of bytes used to write out the value
 */
template <class OutputStream, class T>
typename std::enable_if<std::is_enum<T>::value, uint64_t>::type
write(OutputStream& stream, T value)
{
    auto val = static_cast<typename std::underlying_type<T>::type>(value);
    return write(stream, val);
}

/**
 * Writes an identifier type in a packed representation. This just uses
 * whatever packed representation its underlying type has.
 *
 * @param stream The stream to write to
 * @param value The value to write
 * @return the number of bytes used to write out the value
 */
template <class OutputStream, class Tag, class T>
uint64_t write(OutputStream& stream, const util::identifier<Tag, T>& value)
{
    return write(stream, static_cast<const T&>(value));
}

/**
 * Reads an unsigned integer from its packed representation.
 *
 * @param stream The stream to read from
 * @param value The element to write into
 * @return the number of bytes read
 */
template <class InputStream, class T>
typename std::enable_if<!std::is_floating_point<T>::value
                            && std::is_unsigned<T>::value
                            && !std::is_same<T, bool>::value,
                        uint64_t>::type
read(InputStream& stream, T& value)
{
    value = 0;
    uint64_t size = 0;
    uint8_t byte;
    do
    {
        byte = static_cast<uint8_t>(stream.get());
        value |= static_cast<T>(byte & 127) << (7 * size);
        ++size;
    } while (byte & 128);
    return size;
}

/**
 * Reads a boolean from its packed representation.
 *
 * @param stream The stream to read from
 * @param value The value to write into
 * @return the number of bytes read
 */
template <class InputStream, class T>
typename std::enable_if<std::is_same<T, bool>::value, uint64_t>::type
read(InputStream& stream, T& value)
{
    uint8_t byte;
    auto bytes = read(stream, byte);
    value = byte > 0;
    return bytes;
}

/**
 * Reads a signed integer from its packed representation. This does the
 * reverse of zig-zag encoding to convert from a unsigned integer to a
 * signed integer after reading from the file.
 *
 * @see http://stackoverflow.com/questions/2210923/zig-zag-decoding
 *
 * @param stream The stream to read from
 * @param value The element to write into
 * @return the number of bytes read
 */
template <class InputStream, class T>
typename std::enable_if<!std::is_floating_point<T>::value
                            && std::is_signed<T>::value,
                        uint64_t>::type
read(InputStream& stream, T& value)
{
    typename std::make_unsigned<T>::type elem;
    auto bytes = read(stream, elem);

    value = (elem >> 1) ^ (-(elem & 1));

    return bytes;
}

/**
 * Reads a double from its packed representation.
 *
 * @param stream The stream to read from
 * @param value The element to write into
 * @return the number of bytes read
 */
template <class InputStream, class T>
typename std::enable_if<std::is_floating_point<T>::value, uint64_t>::type
read(InputStream& stream, T& value)
{
    int64_t mantissa;
    int64_t exponent;

    auto bytes = read(stream, mantissa);
    bytes += read(stream, exponent);
    value = static_cast<T>(mantissa * std::pow(2.0, exponent));
    return bytes;
}

/**
 * Reads a string from its packed representation.
 *
 * @param stream The stream to read from
 * @param value The element to write into
 * @return the number of bytes read
 */
template <class InputStream>
uint64_t read(InputStream& stream, std::string& value)
{
    value.clear();
    for (auto c = stream.get(); c != '\0'; c = stream.get())
        value += static_cast<char>(c);
    return value.size() + 1;
}

/**
 * Reads an enum from its packed representation. This reads an integer of
 * the underlying type and then sets the enum accordingly.
 *
 * @param stream The stream to read from
 * @param value The element to write to
 * @return the number of bytes read
 */
template <class InputStream, class T>
typename std::enable_if<std::is_enum<T>::value, uint64_t>::type
read(InputStream& stream, T& value)
{
    typename std::underlying_type<T>::type val;
    auto size = read(stream, val);
    value = static_cast<T>(val);
    return size;
}

/**
 * Reads an identifier type from a packed representation. This just uses
 * whatever packed representation its underlying type has.
 *
 * @param stream The stream to write to
 * @param value The value to write
 * @return the number of bytes used to write out the value
 */
template <class InputStream, class Tag, class T>
uint64_t read(InputStream& stream, util::identifier<Tag, T>& value)
{
    return read(stream, static_cast<T&>(value));
}

/**
 * Convenience function for reading a value from packed representation.
 * This assumes that T is default constructable and that you don't require
 * knowing the number of bytes that were read.
 */
template <class T, class InputStream>
T read(InputStream& stream)
{
    T val;
    read(stream, val);
    return val;
}
}
}
}
#endif
