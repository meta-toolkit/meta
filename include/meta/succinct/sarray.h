/**
 * @file sarray.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SUCCINCT_SARRAY_H_
#define META_SUCCINCT_SARRAY_H_

#include "meta/config.h"
#include "meta/io/filesystem.h"
#include "meta/succinct/broadword.h"
#include "meta/succinct/darray.h"

namespace meta
{
namespace succinct
{

namespace sarray_detail
{
inline std::string low_file(const std::string& prefix)
{
    return prefix + "/sarray.low.bin";
}

inline std::string high_file(const std::string& prefix)
{
    return prefix + "/sarray.high.bin";
}

inline std::string num_bits_file(const std::string& prefix)
{
    return prefix + "/sarray.high.num_bits.bin";
}
}

/**
 * Builder for the high and low bits storage for the sarray succinct data
 * structure.
 */
class sarray_builder
{
  public:
    sarray_builder(const std::string& prefix, uint64_t num_ones,
                   uint64_t num_bits);

    void operator()(uint64_t one_pos);

    ~sarray_builder();

  private:
    using builder_type = bit_vector_builder<detail::ostream_word_writer>;

    std::ofstream low_stream_;
    std::ofstream high_stream_;
    std::ofstream nb_stream_;
    std::unique_ptr<builder_type> low_builder_;
    std::unique_ptr<builder_type> high_builder_;
    uint8_t low_bits_;
    uint64_t low_mask_;
    const uint64_t num_ones_;
    uint64_t num_calls_ = 0;
    uint64_t curr_high_word_ = 0;
    uint64_t high_word_idx_ = 0;
    uint64_t high_word_pos_ = 0;
};

/**
 * Storage class for the high and low bits of the sarray structure. To
 * query, you need to construct/load the corresponding sarray_rank or
 * sarray_select objects.
 */
class sarray
{
  public:
    sarray(const std::string& prefix);

    bit_vector_view high_bits() const;

    bit_vector_view low_bits() const;

    /**
     * @return the number of low bits that were stored per number
     */
    uint8_t num_low_bits() const;

  private:
    util::disk_vector<uint64_t> high_bits_;
    util::disk_vector<uint64_t> low_bits_;
    uint64_t high_bit_count_;
    uint8_t num_low_bits_; // not the total count, but per number!
};

/**
 * Query class for rank queries on an sarray succinct data structure.
 */
class sarray_rank
{
  public:
    sarray_rank(const std::string& prefix, const sarray& sarr);

    uint64_t rank(uint64_t i) const;

    uint64_t size() const;

  private:
    const sarray* sarray_;
    darray0 high_bit_zeroes_;
};

/**
 * Query class for select queries on an sarray succinct data structure.
 */
class sarray_select
{
  public:
    sarray_select(const std::string& prefix, const sarray& sarr);

    uint64_t select(uint64_t i) const;

    uint64_t size() const;

  private:
    const sarray* sarray_;
    darray1 high_bit_ones_;
};

/**
 *
 * A builder for the sarray succinct data structure from Okanohara and
 * Sadakane for answering rank queries on sparse bit arrays.
 *
 * @see http://arxiv.org/abs/cs/0610001
 *
 * Constructs an sarray over the given positions, writing files out to
 * the folder denoted by prefix. The positions must be sorted and must
 * be <= total_bits.
 */
template <class ForwardIterator>
sarray make_sarray(const std::string& prefix, ForwardIterator begin,
                   ForwardIterator end, uint64_t total_bits)
{
    {
        filesystem::make_directory(prefix);
        auto num_ones = static_cast<uint64_t>(std::distance(begin, end));
        sarray_builder builder{prefix, num_ones, total_bits};

        for (; begin != end; ++begin)
            builder(*begin);
    }
    return {prefix};
}
}
}
#endif
