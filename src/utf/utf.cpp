/**
 * @file utf.cpp
 * @author Chase Geigle
 */

#include <array>
#include <stdexcept>
#include <unicode/brkiter.h>
#include <unicode/uchar.h>
#include <unicode/uclean.h>
#include <unicode/unistr.h>
#include <unicode/utf8.h>
#include <unicode/translit.h>

#include "detail.h"
#include "meta/util/pimpl.tcc"
#include "meta/utf/utf.h"

namespace meta
{
namespace utf
{
namespace detail
{
void utf8_append_codepoint(std::string& dest, int32_t codepoint)
{
    std::array<uint8_t, U8_MAX_LENGTH> buf;
    int32_t len = 0;
    UBool err = FALSE;
    // ICU has some conversions within this macro, which we can't control
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    U8_APPEND(&buf[0], len, U8_MAX_LENGTH, codepoint, err);
#pragma GCC diagnostic pop
    if (err)
        throw std::runtime_error{"failed to add codepoint to string"};
    dest.append(reinterpret_cast<char*>(&buf[0]),
                static_cast<std::size_t>(len));
}

int32_t utf8_next_codepoint(const char* str, int32_t& idx, int32_t length)
{
    int32_t codepoint;
    U8_NEXT(str, idx, length, codepoint);
    return codepoint;
}
}

std::string to_utf8(const std::string& str, const std::string& charset)
{
    icu_handle::get();
    icu::UnicodeString u16str{str.c_str(), charset.c_str()};
    return icu_to_u8str(u16str);
}

std::u16string to_utf16(const std::string& str, const std::string& charset)
{
    static_assert(sizeof(char16_t) == sizeof(UChar),
                  "Invalid UChar definition in ICU");
    icu_handle::get();
    icu::UnicodeString icu_str{str.c_str(), charset.c_str()};
    return icu_to_u16str(icu_str);
}

std::string to_utf8(const std::u16string& str)
{
    icu_handle::get();
    static_assert(sizeof(char16_t) == sizeof(UChar),
                  "Invalid UChar definition in ICU");
    // looks dangerous, but isn't: UChar is guaranteed to be a 16-bit
    // integer type, so all we're doing here is going between signed vs.
    // unsigned
    auto buf = reinterpret_cast<const UChar*>(str.c_str());
    icu::UnicodeString u16str{buf};
    return icu_to_u8str(u16str);
}

std::u16string to_utf16(const std::string& str)
{
    icu_handle::get();
    auto icu_str = icu::UnicodeString::fromUTF8(str);
    return icu_to_u16str(icu_str);
}

std::string tolower(const std::string& str)
{
    return transform(str, [](uint32_t cp)
                     {
                         return u_tolower(static_cast<UChar32>(cp));
                     });
}

std::string toupper(const std::string& str)
{
    return transform(str, [](uint32_t cp)
                     {
                         return u_toupper(static_cast<UChar32>(cp));
                     });
}

std::string foldcase(const std::string& str)
{
    return transform(str, [](uint32_t cp)
                     {
                         return u_foldCase(static_cast<UChar32>(cp),
                                           U_FOLD_CASE_DEFAULT);
                     });
}

bool isalpha(uint32_t codepoint)
{
    return u_isalpha(static_cast<UChar32>(codepoint));
}

bool isblank(uint32_t codepoint)
{
    return u_isblank(static_cast<UChar32>(codepoint));
}

bool isspace(uint32_t codepoint)
{
    return u_isUWhiteSpace(static_cast<int32_t>(codepoint));
}

uint64_t length(const std::string& str)
{
    const char* s = str.c_str();
    auto length = static_cast<int32_t>(str.length());
    uint64_t count = 0;
    for (int32_t i = 0; i < length;)
    {
        UChar32 c;
        U8_NEXT(s, i, length, c);
        ++count;
    }
    return count;
}
}
}
