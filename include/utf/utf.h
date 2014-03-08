/**
 * @file utf.h
 * @author Chase Geigle
 */

#ifndef _META_UTF8_H_
#define _META_UTF8_H_

#include <functional>
#include <string>

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

/**
 * Lowercases a utf8 string.
 */
std::string tolower(const std::string& str);

/**
 * Uppercases a utf8 string.
 */
std::string toupper(const std::string& str);

/**
 * Folds the case of a utf8 string. This is like lowercase, but a bit more
 * general.
 */
std::string foldcase(const std::string& str);

/**
 * Transliterates a utf8 string, using the rules defined in ICU.
 * @see http://userguide.icu-project.org/transforms
 */
std::string transform(const std::string& str, const std::string& id);

/**
 * Removes characters that match the given function.
 */
std::string remove_if(const std::string& str,
                      std::function<bool(uint32_t)> pred);

/**
 * Gets the number of code points in a utf8 string.
 */
uint64_t length(const std::string& str);

/**
 * Determines if a code point is a letter character.
 */
bool isalpha(uint32_t codepoint);
}
}

#endif
