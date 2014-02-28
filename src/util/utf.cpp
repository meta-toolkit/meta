/**
 * @file utf.cpp
 * @author Chase Geigle
 */

#include <stdexcept>
#include <unicode/unistr.h>
#include <unicode/uclean.h>

#include "util/utf.h"

namespace meta
{
namespace utf
{

namespace
{

class icu_handle
{
    icu_handle()
    {
        auto status = U_ZERO_ERROR;
        u_init(&status);
        if (!U_SUCCESS(status))
            throw std::runtime_error{"Failed to initialize icu"};
    }

    public:
        static icu_handle& get()
        {
            static icu_handle handle;
            return handle;
        }

        ~icu_handle()
        {
            u_cleanup();
        }
};

std::u16string icu_to_u16str(const icu::UnicodeString& icu_str)
{
    std::u16string u16str;
    u16str.resize(icu_str.length());
    auto status = U_ZERO_ERROR;
    // looks dangerous, actually isn't: UChar is guaranteed to be a 16-bit
    // integer type, so all we're doing here is going between signed vs.
    // unsigned
    icu_str.extract(reinterpret_cast<UChar*>(&u16str[0]), u16str.length(), status);
    return u16str;
}

std::string icu_to_u8str(const icu::UnicodeString& icu_str)
{
    std::string u8str;
    icu_str.toUTF8String(u8str);
    return u8str;
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

}
}
