/**
 * @file utf.h
 * @author Chase Geigle
 */

#ifndef _META_UTF8_H_
#define _META_UTF8_H_

#include <string>
#include <stdexcept>
#include <unicode/unistr.h>
#include <unicode/uclean.h>

namespace meta
{
namespace utf
{

/**
 * Converts a string from the given charset to utf8.
 * @param str the string to convert
 * @param charset the charset of the given string
 */
std::string to_utf8(const std::string& str, const std::string& charset);

/**
 * Converts a string fro the given charset to utf16.
 * @param str the string to convert
 * @param charset the charset of the given string
 */
std::u16string to_utf16(const std::string& str, const std::string& charset);

/**
 * Converts a string from utf16 to utf8.
 *
 * @param str the string to convert
 */
std::string to_utf8(const std::u16string& str);

/**
 * Converts a string from utf8 to utf16.
 *
 * @param str the string to convert
 */
std::u16string to_utf16(const std::string& str);

}
}

#endif
