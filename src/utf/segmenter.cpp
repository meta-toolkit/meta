/**
 * @file segmenter.cpp
 * @author Chase Geigle
 */

#include <unicode/brkiter.h>

#include "detail.h"
#include "meta/utf/segmenter.h"
#include "meta/util/pimpl.tcc"

namespace meta
{
namespace utf
{

/**
 * Implementation class for the segmenter.
 */
class segmenter::impl
{
  public:
    /**
     * Constructs a new impl.
     */
    impl()
    {
        auto status = U_ZERO_ERROR;
        const auto& locale = icu::Locale::getUS();
        sentence_iter_.reset(
            icu::BreakIterator::createSentenceInstance(locale, status));
        word_iter_.reset(
            icu::BreakIterator::createWordInstance(locale, status));
        if (!U_SUCCESS(status))
            throw std::runtime_error{"failed to create segmenter"};
    }

    /**
     * Constructs a new impl based on the locale parameters.
     */
    impl(const std::string& language,
         const util::optional<std::string>& country)
    {
        icu::Locale locale(language.c_str(),
                           country ? country->c_str() : nullptr);
        if (locale.isBogus())
            throw std::runtime_error{"failed to create locale"};

        auto status = U_ZERO_ERROR;
        sentence_iter_.reset(
            icu::BreakIterator::createSentenceInstance(locale, status));
        word_iter_.reset(
            icu::BreakIterator::createWordInstance(locale, status));
        if (!U_SUCCESS(status))
            throw std::runtime_error{"failed to create segmenter"};
    }

    /**
     * Copy constructs an impl.
     * @param other The impl to copy.
     */
    impl(const impl& other)
        : text_{other.text_},
          sentence_iter_{other.sentence_iter_->clone()},
          word_iter_{other.word_iter_->clone()}
    {
        // nothing
    }

    /// Defaulted move constructor.
    impl(impl&&) = default;

    /**
     * Sets the content of the segmenter.
     * @param str The content to be set
     */
    void set_content(util::string_view str)
    {
        text_ = str;
    }

    /**
     * @param begin The beginning index
     * @param end The ending index
     * @return the substring between begin and end
     */
    util::string_view substr(int32_t begin, int32_t end) const
    {
        assert(end >= begin);
        return text_.substr(static_cast<std::size_t>(begin),
                            static_cast<std::size_t>(end - begin));
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
     * @return a vector of segments representing sentences
     */
    std::vector<segment> sentences() const
    {
        return segments(0, static_cast<int32_t>(text_.length()),
                        segment_t::SENTENCES);
    }

    /**
     * Segments the entire content into words.
     * @return a vector of segments representing words
     */
    std::vector<segment> words() const
    {
        return segments(0, static_cast<int32_t>(text_.length()),
                        segment_t::WORDS);
    }

    /**
     * Generic segmentation method that operates on the substring between
     * the given indices, using the given strategy for segmenting that
     * substring.
     *
     * @param first The index of the beginning of the string to work on
     * @param last The index of the end of the string to work on
     * @param type The type of segmentation to perform
     * @return a vector of segments (whose meaning depends on `type`)
     */
    std::vector<segment> segments(int32_t first, int32_t last,
                                  segment_t type) const
    {
        std::vector<segment> results;
        icu::BreakIterator* iter;
        if (type == segment_t::SENTENCES)
            iter = sentence_iter_.get();
        else if (type == segment_t::WORDS)
            iter = word_iter_.get();
        else
            throw std::runtime_error{"Unknown segmentation type"};

        auto status = U_ZERO_ERROR;
        UText utxt = UTEXT_INITIALIZER;
        utext_openUTF8(&utxt, text_.data() + first, last - first, &status);
        if (!U_SUCCESS(status))
        {
            std::string err = "Failed to open UText: ";
            err += u_errorName(status);
            throw std::runtime_error{err};
        }

        iter->setText(&utxt, status);
        if (!U_SUCCESS(status))
        {
            utext_close(&utxt);
            std::string err = "Failed to setText: ";
            err += u_errorName(status);
            throw std::runtime_error{err};
        }

        auto start = iter->first();
        auto end = iter->next();
        while (end != icu::BreakIterator::DONE)
        {
            results.emplace_back(first + start, first + end);
            start = end;
            end = iter->next();
        }
        utext_close(&utxt);
        return results;
    }

  private:
    /// A view over the utf8 string we are segmenting
    util::string_view text_;
    /// A pointer to a sentence break iterator
    std::unique_ptr<icu::BreakIterator> sentence_iter_;
    /// A pointer to a word break iterator
    std::unique_ptr<icu::BreakIterator> word_iter_;
};

segmenter::segmenter()
{
    icu_handle::get();
}

segmenter::segmenter(const std::string& language,
                     const util::optional<std::string>& country)
    : impl_{language, country}
{
    // nothing
}

segmenter::segmenter(const segmenter& other) : impl_{*other.impl_}
{
    // nothing
}

segmenter::~segmenter() = default;

void segmenter::set_content(util::string_view str)
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

util::string_view segmenter::content(const segment& seg) const
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
