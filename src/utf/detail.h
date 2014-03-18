/**
 * @file detail.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_UTF_DETAIL_H_
#define META_UTF_DETAIL_H_

#include <array>
#include <stdexcept>

#include <unicode/uclean.h>
#include <unicode/unistr.h>

namespace meta
{
namespace utf
{

/**
 * Internal class that ensures that ICU cleans up all of its
 * "still-reachable" memory before program termination.
 */
class icu_handle
{
  private:
    icu_handle()
    {
        auto status = U_ZERO_ERROR;
        u_init(&status);
        if (!U_SUCCESS(status))
            throw std::runtime_error{"Failed to initialize icu"};
    }

  public:
    /**
     * All functions that use ICU should call this function first to
     * ensure that the handle is instantiated for later cleanup.
     */
    inline static icu_handle& get()
    {
        static icu_handle handle;
        return handle;
    }

    /**
     * Destructor. Invokes the ICU cleanup method.
     */
    ~icu_handle()
    {
        u_cleanup();
    }
};

/**
 * Helper method that converts an ICU string to a std::u16string.
 *
 * @param icu_str The ICU string to be converted
 * @return a std::u16string from the given ICU string
 */
inline std::u16string icu_to_u16str(const icu::UnicodeString& icu_str)
{
    std::u16string u16str;
    u16str.resize(icu_str.length());
    auto status = U_ZERO_ERROR;
    // looks dangerous, actually isn't: UChar is guaranteed to be a 16-bit
    // integer type, so all we're doing here is going between signed vs.
    // unsigned
    icu_str.extract(reinterpret_cast<UChar*>(&u16str[0]), u16str.length(),
                    status);
    return u16str;
}

/**
 * Helper method that converts an ICU string to a std::string in utf8.
 *
 * @param icu_str The ICU string to be converted
 * @return a std::string in utf8 from the given ICU string
 */
inline std::string icu_to_u8str(const icu::UnicodeString& icu_str)
{
    std::string u8str;
    icu_str.toUTF8String(u8str);
    return u8str;
}

/**
 * Helper method that appends a UTF-32 codepoint to the given utf8 string.
 * @param dest The string to append the codepoint to
 * @param codepoint The UTF-32 codepoint to append
 */
inline void utf8_append_codepoint(std::string& dest, uint32_t codepoint)
{
    std::array<uint8_t, U8_MAX_LENGTH> buf;
    int32_t len = 0;
    UBool err = FALSE;
    U8_APPEND(&buf[0], len, U8_MAX_LENGTH, codepoint, err);
    if (err)
        throw std::runtime_error{"failed to add codepoint to string"};
    dest.append(reinterpret_cast<char*>(&buf[0]), len);
}
}
}
#endif
