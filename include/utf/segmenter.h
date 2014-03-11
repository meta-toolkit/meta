/**
 * @file segmenter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_UTF_SEGMENTER_H_
#define META_UTF_SEGMENTER_H_

#include <string>
#include <vector>
#include "util/pimpl.h"

namespace meta
{
namespace utf
{

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
     * Copy constructs a segmenter.
     */
    segmenter(const segmenter&);

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
