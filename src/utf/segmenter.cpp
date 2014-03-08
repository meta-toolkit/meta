/**
 * @file segmenter.cpp
 * @author Chase Geigle
 */

#include <unicode/brkiter.h>

#include "detail.h"
#include "utf/segmenter.h"
#include "util/pimpl.tcc"

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
