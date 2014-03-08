/**
 * @file utf.cpp
 * @author Chase Geigle
 */

#include <array>
#include <stdexcept>
#include <unicode/brkiter.h>
#include <unicode/uclean.h>
#include <unicode/unistr.h>
#include <unicode/translit.h>

#include "util/pimpl.tcc"
#include "utf/utf.h"

namespace meta
{
namespace utf
{

namespace
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
    icu_str.extract(reinterpret_cast<UChar*>(&u16str[0]), u16str.length(),
                    status);
    return u16str;
}

std::string icu_to_u8str(const icu::UnicodeString& icu_str)
{
    std::string u8str;
    icu_str.toUTF8String(u8str);
    return u8str;
}

void utf8_append_codepoint(std::string& dest, uint32_t codepoint)
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
    const char* s = str.c_str();
    std::string result;
    result.reserve(str.length());
    int32_t length = str.length();
    for (int32_t i = 0; i < length;)
    {
        UChar32 codepoint;
        U8_NEXT(s, i, length, codepoint);
        auto lower = u_tolower(codepoint);
        utf8_append_codepoint(result, lower);
    }
    return result;
}

std::string toupper(const std::string& str)
{
    const char* s = str.c_str();
    std::string result;
    result.reserve(str.length());
    int32_t length = str.length();
    for (int32_t i = 0; i < length;)
    {
        UChar32 codepoint;
        U8_NEXT(s, i, length, codepoint);
        auto upper = u_toupper(codepoint);
        utf8_append_codepoint(result, upper);
    }
    return result;
}

std::string foldcase(const std::string& str)
{

    const char* s = str.c_str();
    std::string result;
    result.reserve(str.length());
    int32_t length = str.length();
    for (int32_t i = 0; i < length;)
    {
        UChar32 codepoint;
        U8_NEXT(s, i, length, codepoint);
        auto folded = u_foldCase(codepoint, U_FOLD_CASE_DEFAULT);
        utf8_append_codepoint(result, folded);
    }
    return result;
}

std::string remove_if(const std::string& str,
                      std::function<bool(uint32_t)> pred)
{
    std::string result;
    const char* s = str.c_str();
    int32_t length = str.length();
    for (int32_t i = 0; i < length;)
    {
        UChar32 codepoint;
        U8_NEXT(s, i, length, codepoint);
        if (pred(codepoint))
          continue;
        utf8_append_codepoint(result, codepoint);
    }
    return result;
}

bool isalpha(uint32_t codepoint)
{
    return u_isalpha(codepoint);
}

uint64_t length(const std::string& str)
{
    const char* s = str.c_str();
    int32_t length = str.length();
    uint64_t count = 0;
    for (int32_t i = 0; i < length;)
    {
        UChar32 c;
        U8_NEXT(s, i, length, c);
        ++count;
    }
    return count;
}

/**
 * Implementation class for the transformer.
 */
class transformer::impl
{
  public:
    impl(const std::string& id)
    {
        icu_handle::get();
        auto icu_id = icu::UnicodeString::fromUTF8(id);
        auto status = U_ZERO_ERROR;
        translit_.reset(icu::Transliterator::createInstance(
            icu_id, UTRANS_FORWARD, status));
        if (!translit_ || !U_SUCCESS(status))
            throw std::runtime_error{"failed to create transformer"};
    }

    impl(const impl& other)
        : translit_{other.translit_->clone()}
    {
        // nothing
    }

    impl(impl&&) = default;

    std::string convert(const std::string& str)
    {
        auto icu_str = icu::UnicodeString::fromUTF8(str);
        translit_->transliterate(icu_str);
        return icu_to_u8str(icu_str);
    }

  private:
    std::unique_ptr<icu::Transliterator> translit_;
};

transformer::transformer(const std::string& id)
    : impl_{id}
{
    icu_handle::get();
}

transformer::transformer(const transformer& other)
    : impl_{*other.impl_}
{
    // nothing
}

transformer::transformer(transformer&&) = default;

transformer::~transformer() = default;

std::string transformer::operator()(const std::string& str)
{
    return impl_->convert(str);
}

std::string transform(const std::string& str, const std::string& id)
{
    icu_handle::get();
    transformer trans{id};
    return trans(str);
}

/**
 * Implementation class for the segmenter.
 */
class segmenter::impl
{
  public:
    impl()
    {
        auto status = U_ZERO_ERROR;
        const auto& locale = icu::Locale::getDefault();
        sentence_iter_.reset(
            icu::BreakIterator::createSentenceInstance(locale, status));
        word_iter_.reset(
            icu::BreakIterator::createWordInstance(locale, status));
        if (!U_SUCCESS(status))
            throw std::runtime_error{"failed to create segmenter"};
    }

    impl(const impl& other)
        : u_str_{other.u_str_},
          sentence_iter_{other.sentence_iter_->clone()},
          word_iter_{other.word_iter_->clone()}
    {
        // nothing
    }

    impl(impl&&) = default;

    /**
     * Sets the content of the segmenter.
     */
    void set_content(const std::string& str)
    {
        u_str_ = icu::UnicodeString::fromUTF8(str);
    }

    /**
     * Obtains a utf-8 encoded string by first extracting the utf-16
     * encoded substring between the given indices and converting that
     * substring to utf-8.
     */
    std::string substr(int32_t begin, int32_t end) const
    {
        auto substring = u_str_.tempSubStringBetween(begin, end);
        return icu_to_u8str(substring);
    }

    /**
     * Tag class for the segmentation strategy.
     */
    enum class segment_t
    {
        SENTENCES,
        WORDS
    };

    /**
     * Segments the entire content into sentences.
     */
    std::vector<segment> sentences() const
    {
        return segments(0, u_str_.length(), segment_t::SENTENCES);
    }

    /**
     * Segments the entire content into words.
     */
    std::vector<segment> words() const
    {
        return segments(0, u_str_.length(), segment_t::WORDS);
    }

    /**
     * Generic segmentation method that operates on the substring between
     * the given indices, using the given strategy for segmenting that
     * substring.
     */
    std::vector<segment> segments(int32_t first, int32_t last,
                                  segment_t type) const
    {
        std::vector<segment> results;
        auto status = U_ZERO_ERROR;
        icu::BreakIterator* iter;
        if (type == segment_t::SENTENCES)
            iter = sentence_iter_.get();
        else if (type == segment_t::WORDS)
            iter = word_iter_.get();
        else
            throw std::runtime_error{"Unknown segmentation type"};

        if (!U_SUCCESS(status))
        {
            std::string err = "Failed to segment: ";
            err += u_errorName(status);
            throw std::runtime_error{err};
        }

        iter->setText(u_str_.tempSubStringBetween(first, last));

        auto start = iter->first();
        auto end = iter->next();
        while (end != icu::BreakIterator::DONE)
        {
            results.emplace_back(first + start, first + end);
            start = end;
            end = iter->next();
        }
        return results;
    }

  private:
    icu::UnicodeString u_str_;
    std::unique_ptr<icu::BreakIterator> sentence_iter_;
    std::unique_ptr<icu::BreakIterator> word_iter_;
};

segmenter::segmenter()
{
    icu_handle::get();
}

segmenter::segmenter(const segmenter& other)
    : impl_{*other.impl_}
{
    // nothing
}

segmenter::~segmenter() = default;

void segmenter::set_content(const std::string& str)
{
    impl_->set_content(str);
}

auto segmenter::sentences() const -> std::vector<segment>
{
    return impl_->sentences();
}

auto segmenter::words() const -> std::vector<segment>
{
    return impl_->words();
}

auto segmenter::words(const segment& seg) const -> std::vector<segment>
{
    return impl_->segments(seg.begin_, seg.end_, impl::segment_t::WORDS);
}

std::string segmenter::content(const segment& seg) const
{
    return impl_->substr(seg.begin_, seg.end_);
}

segmenter::segment::segment(int32_t begin, int32_t end)
    : begin_{begin}, end_{end}
{
    // nothing
}
}
}
