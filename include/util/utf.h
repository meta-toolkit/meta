/**
 * @file utf.h
 * @author Chase Geigle
 */

#ifndef _META_UTF8_H_
#define _META_UTF8_H_

#include <string>
#include <stdexcept>
#include <vector>

#include <unicode/unistr.h>
#include <unicode/uclean.h>

#include "util/pimpl.h"

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
 * Transliterates a utf8 string, using the rules defined in ICU.
 * @see http://userguide.icu-project.org/transforms
 */
std::string transform(const std::string& str, const std::string& id);

/**
 * Class that encapsulates transliteration of unicode strings.
 * @see http://userguide.icu-project.org/transforms
 */
class transformer
{
  public:
    /**
     * Constructs a new transformer.
     *
     * @param id the id of the transliteration rule as defined by ICU
     */
    transformer(const std::string& id);

    /**
     * Copy constructor.
     */
    transformer(const transformer& other);

    /**
     * Move constructor.
     */
    transformer(transformer&& other);

    /**
     * Destructor for the transformer.
     */
    ~transformer();

    /**
     * Transforms the given utf8 string.
     */
    std::string operator()(const std::string& str);

  private:
    class impl;
    util::pimpl<impl> impl_;
};

/**
 * Class that encapsulates segmenting unicode strings. Supports segmenting
 * sentences as well as words.
 */
class segmenter
{
  public:
    /**
     * Represents a segment within a unicode string. Created by the
     * segmenter class.
     */
    class segment
    {
      public:
        segment(int32_t begin, int32_t end);

      private:
        friend segmenter;
        // using int32_t here because of ICU, which accepts only int32_t as
        // its indexes
        int32_t begin_;
        int32_t end_;
    };

    /**
     * Constructs a segmenter. An instance of segmenter may be used to
     * segment many different unicode strings, and it is encouraged to
     * re-use one if you are segmenting many strings.
     */
    segmenter();

    /**
     * Destructor for segmenter.
     */
    ~segmenter();

    /**
     * Resets the content of the segmenter to the given string.
     *
     * @param str a utf-8 string that should be segmented
     */
    void set_content(const std::string& str);

    /**
     * Segments the current content into sentences by following the
     * unicode segmentation standard.
     */
    std::vector<segment> sentences() const;

    /**
     * Segments the current content into words by following the unicode
     * segmentation standard.
     */
    std::vector<segment> words() const;

    /**
     * Segments a given segment into words by following the unicode
     * segmentation standard. Typically, this would be used to further
     * segment a sentence segment into its constituent words.
     *
     * @param seg the segment to sub-segment into words
     */
    std::vector<segment> words(const segment& seg) const;

    /**
     * Gets the content associated with a given segment as a utf-8 encoded
     * string.
     *
     * @param seg the segment to get content for
     */
    std::string content(const segment& seg) const;

  private:
    class impl;
    util::pimpl<impl> impl_;
};
}
}

#endif
