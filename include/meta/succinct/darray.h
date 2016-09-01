/**
 * @file darray.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SUCCINCT_DARRAY_H_
#define META_SUCCINCT_DARRAY_H_

#include <fstream>
#include <iostream>

#include "meta/config.h"
#include "meta/io/binary.h"
#include "meta/io/filesystem.h"
#include "meta/io/packed.h"
#include "meta/succinct/bit_vector.h"
#include "meta/succinct/broadword.h"
#include "meta/util/disk_vector.h"
#include "meta/util/pimpl.h"

namespace meta
{
namespace succinct
{

struct word_identity
{
    uint64_t operator()(uint64_t word) const
    {
        return word;
    }
};

struct word_inverse
{
    uint64_t operator()(uint64_t word) const
    {
        return ~word;
    }
};

namespace darray_detail
{
inline std::string blocks_file(const std::string& prefix)
{
    return prefix + "/darray.blocks.bin";
}

inline std::string sub_blocks_file(const std::string& prefix)
{
    return prefix + "/darray.subblocks.bin";
}

inline std::string explicit_positions_file(const std::string& prefix)
{
    return prefix + "/darray.explicit.bin";
}

inline std::string num_ones_file(const std::string& prefix)
{
    return prefix + "/darray.num_ones.bin";
}

/**
 * \f$L\f$ from the paper, the number of ones within each block
 */
const static constexpr uint64_t ones_per_block = 1 << 10;
/**
 * \f$L_2\f$ from the paper, the maximum bit span a block can have
 * before all of its ones' positions are recorded explicitly.
 */
const static constexpr uint64_t max_distance = 1 << 16;
/**
 * \f$L_3\f$ from the paper: within each block with distance below
 * \f$L_3\f$, store the position of every \f$L_3\f$-th on.
 */
const static constexpr uint64_t sub_block_stride = 32;
}

/**
 * A builder for the darray succinct data structure from Okanohara and
 * Sadakane for answering select queries on dense bit arrays of length
 * \f$n\f$ where the number of ones \f$m\f$ is about \f$n/2\f$.
 *
 * @see http://arxiv.org/abs/cs/0610001
 */
template <class WordReader = word_identity>
class darray_builder
{
  public:
    /**
     * Constructs a darray over the given bit vector, writing output into
     * the folder denoted by prefix.
     *
     * Adapted from code by Giuseppe Ottaviano and released under the
     * Apache 2.0 license.
     *
     * @see https://github.com/ot/succinct/blob/master/darray.hpp
     * @see https://github.com/ot/succinct/blob/master/LICENSE
     */
    darray_builder(const std::string& prefix, bit_vector_view bvv)
    {
        using namespace darray_detail;
        filesystem::make_directory(prefix);

        /**
         * Output stream for an array that stores the positions of the
         * \f$iL + 1\f$-th one if the block size was less than \f$L_2\f$, and
         * a negative number indicating the index into the explicit array
         * otherwise.
         */
        std::ofstream blocks{blocks_file(prefix), std::ios::binary};
        /**
         * Output stream for an array that stores every \f$L_3\f$-th one
         * within blocks of size less than \f$L_2\f$ (and some undefined
         * number for every \f$L_3\f$-th one within larger blocks).
         */
        std::ofstream sub_blocks{sub_blocks_file(prefix), std::ios::binary};
        /**
         * Output stream for storing the explicit positions of ones for the
         * blocks that were larger than \f$L_2\f$.
         */
        std::ofstream explicit_positions{explicit_positions_file(prefix),
                                         std::ios::binary};

        uint64_t num_ones = 0;
        std::vector<uint64_t> current_block;
        current_block.reserve(ones_per_block);
        auto data = bvv.data();
        for (uint64_t word_pos = 0; word_pos < data.size(); ++word_pos)
        {
            // reverse the word if needed
            auto word = WordReader{}(data[word_pos]);
            uint64_t bit_pos = word_pos * 64;

            // until we've read every bit in the bit vector, or we've run
            // out of ones in the current word
            while (bit_pos < bvv.size() && word)
            {
                // find the position of the next 1
                auto one_pos = broadword::lsb(word);
                bit_pos += one_pos;
                word >>= one_pos;

                // record the position of the one and flush the block if
                // needed
                current_block.push_back(bit_pos);
                if (current_block.size() == ones_per_block)
                {
                    flush_block(current_block, blocks, sub_blocks,
                                explicit_positions);
                }

                // move everything forward past the one
                ++bit_pos;
                word >>= 1;

                // record that we've seen another one
                ++num_ones;
            }
        }

        if (!current_block.empty())
        {
            flush_block(current_block, blocks, sub_blocks, explicit_positions);
        }

        // force the file to have data
        if (num_explicit_ones_ == 0)
            io::write_binary(explicit_positions, static_cast<uint64_t>(-1));

        std::ofstream num_ones_file{darray_detail::num_ones_file(prefix),
                                    std::ios::binary};
        io::packed::write(num_ones_file, num_ones);
    }

  private:
    /**
     * Flushes a completed block of ones to disk.
     */
    void flush_block(std::vector<uint64_t>& current_block, std::ostream& blocks,
                     std::ostream& sub_blocks, std::ostream& explicit_positions)
    {
        using namespace darray_detail;

        // if the block is larger than L_2, store every one position
        // explicitly in S_l
        if (current_block.back() - current_block.front() > max_distance)
        {
            io::write_binary(blocks, -num_explicit_ones_ - 1);
            num_explicit_ones_ += static_cast<int64_t>(current_block.size());

            for (const auto& pos : current_block)
            {
                io::write_binary(explicit_positions, pos);
            }

            for (std::size_t i = 0; i < current_block.size();
                 i += sub_block_stride)
            {
                io::write_binary(sub_blocks, static_cast<uint16_t>(-1));
            }
        }
        // otherwise, store every L_3-th one in the block in S_s
        else
        {
            io::write_binary(blocks, static_cast<int64_t>(current_block[0]));

            for (std::size_t i = 0; i < current_block.size();
                 i += sub_block_stride)
            {
                auto offset = static_cast<uint16_t>(current_block[i]
                                                    - current_block[0]);
                assert(i == 0 || offset > 0);
                io::write_binary(sub_blocks, offset);
            }
        }
        current_block.clear();
    }

    /**
     * The total number of one positions that have been written to the
     * explicit positions array.
     */
    int64_t num_explicit_ones_ = 0;
};

template <class WordReader = word_identity>
class darray
{
  public:
    /**
     * Loads or creates a darray, stored in files in the given prefix
     * (folder).
     *
     * @param prefix The folder containing the output from a
     * darray_builder
     * @param bvv The bit vector that this darray should index over
     */
    darray(const std::string& prefix, bit_vector_view bvv)
    {
        if (!is_valid(prefix))
        {
            darray_builder<WordReader>{prefix, bvv};
        }
        impl_ = make_unique<impl>(prefix, bvv);
    }

    darray(darray&& other) : impl_{std::move(other.impl_)}
    {
        // nothing
    }

    darray& operator=(darray&& rhs)
    {
        impl_ = std::move(rhs.impl_);
        return *this;
    }

    /**
     * Determines the position of the \f$i\f$-th one in the bit vector.
     */
    uint64_t select(uint64_t i) const
    {
        return impl_->select(i);
    }

    /**
     * @return the number of indexed positions in the vector
     */
    uint64_t num_positions() const
    {
        return impl_->num_ones;
    }

  private:
    static bool is_valid(const std::string& prefix)
    {
        return filesystem::file_exists(darray_detail::blocks_file(prefix))
               && filesystem::file_exists(
                      darray_detail::sub_blocks_file(prefix))
               && filesystem::file_exists(
                      darray_detail::explicit_positions_file(prefix))
               && filesystem::file_exists(darray_detail::num_ones_file(prefix));
    }

    struct impl
    {
        impl(const std::string& prefix, bit_vector_view bv)
            : bvv{bv},
              blocks{darray_detail::blocks_file(prefix)},
              sub_blocks{darray_detail::sub_blocks_file(prefix)},
              explicit_positions{darray_detail::explicit_positions_file(prefix)}
        {
            std::ifstream num_ones_file{darray_detail::num_ones_file(prefix),
                                        std::ios::binary};
            io::packed::read(num_ones_file, num_ones);
        }

        uint64_t select(uint64_t i) const
        {
            using namespace darray_detail;

#if DEBUG
            if (META_UNLIKELY(i >= num_ones))
                throw std::out_of_range{"index out of range in select query"};
#endif

            auto block_idx = i / ones_per_block;
            if (blocks[block_idx] < 0)
            {
                // this was one of the blocks that was stored explicitly
                auto block_start
                    = static_cast<uint64_t>(-blocks[block_idx] - 1);
                return explicit_positions[block_start + i % ones_per_block];
            }

            // otherwise, look up the closest L_3-th one and do a
            // sequential scan from there
            auto subblock_idx = i / sub_block_stride;
            auto one_count = i % sub_block_stride;
            auto start_pos = static_cast<uint64_t>(blocks[block_idx])
                             + sub_blocks[subblock_idx];

            auto words = bvv.data();
            if (one_count == 0)
                return start_pos;

            auto word_idx = start_pos / 64;
            auto word_pos = start_pos % 64;
            auto word = WordReader{}(words[word_idx])
                        & (static_cast<uint64_t>(-1) << word_pos);

            while (true)
            {
                auto popcount = broadword::popcount(word);
                if (one_count < popcount)
                    break;
                one_count -= popcount;
                word = WordReader{}(words[++word_idx]);
            }

            return 64 * word_idx + broadword::select_in_word(word, one_count);
        }

        /**
         * The bit vector view the darray indexes over.
         */
        bit_vector_view bvv;
        /**
         * An array that stores the positions of the \f$iL + 1\f$-th one if
         * the block size was less than \f$L_2\f$, and a negative number
         * indicating the index into the explicit array otherwise.
         */
        util::disk_vector<int64_t> blocks;
        /**
         * An array that stores every \f$L_3\f$-th one within blocks of
         * size less than \f$L_2\f$ (and some undefined number for every
         * \f$L_3\f$-th one within larger blocks).
         */
        util::disk_vector<uint16_t> sub_blocks;
        /**
         * An array storing the explicit positions of ones for the blocks
         * that were larger than \f$L_2\f$.
         */
        util::disk_vector<uint64_t> explicit_positions;
        /**
         * The total number of ones found during construction.
         */
        uint64_t num_ones;
    };
    std::unique_ptr<impl> impl_;
};

using darray1 = darray<>;
using darray0 = darray<word_inverse>;
}
}
#endif
