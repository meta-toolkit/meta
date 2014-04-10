/**
 * @file segmenter.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
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
        /**
         * Creates a segment.
         *
         * @param begin The starting index of the segment
         * @param end The ending index of the segment
         */
        segment(int32_t begin, int32_t end);

      private:
        friend segmenter;
        // using int32_t here because of ICU, which accepts only int32_t as
        // its indexes
        /// The beginning index of this segment.
        int32_t begin_;
        /// The ending index of this segment.
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
     * @param str A utf-8 string that should be segmented
     */
    void set_content(const std::string& str);

    /**
     * Segments the current content into sentences by following the
     * unicode segmentation standard.
     *
     * @return a vector of segments that represent sentences
     */
    std::vector<segment> sentences() const;

    /**
     * Segments the current content into words by following the unicode
     * segmentation standard.
     *
     * @return a vector of segments that represent words
     */
    std::vector<segment> words() const;

    /**
     * Segments a given segment into words by following the unicode
     * segmentation standard. Typically, this would be used to further
     * segment a sentence segment into its constituent words.
     *
     * @param seg the segment to sub-segment into words
     * @return a vector of segments that represent words
     */
    std::vector<segment> words(const segment& seg) const;

    /**
     * @return the content associated with a given segment as a utf-8
     * encoded string
     * @param seg the segment to get content for
     */
    std::string content(const segment& seg) const;

  private:
    class impl;
    /// A pointer to the implementation class for the segmenter.
    util::pimpl<impl> impl_;
};
}
}
#endif
