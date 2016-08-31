/**
 * @file perfect_hash_builder.tcc
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <algorithm>
#include <fstream>
#include <random>
#include "meta/hashing/hash.h"
#include "meta/hashing/probe_set.h"
#include "meta/io/filesystem.h"
#include "meta/io/moveable_stream.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/succinct/compressed_vector.h"
#include "meta/succinct/sarray.h"
#include "meta/util/array_view.h"
#include "meta/util/disk_vector.h"
#include "meta/util/multiway_merge.h"
#include "meta/util/printing.h"
#include "perfect_hash_builder.h"

namespace meta
{
namespace hashing
{

namespace mph
{
template <class K>
struct bucket_record
{
    std::size_t idx;
    std::vector<K> keys;

    void merge_with(bucket_record&& other)
    {
        std::move(other.keys.begin(), other.keys.end(),
                  std::back_inserter(keys));
        std::vector<K>{}.swap(other.keys);
    }
};

template <class K>
bool operator==(const bucket_record<K>& a, const bucket_record<K>& b)
{
    return a.idx == b.idx;
}

template <class K>
bool operator<(const bucket_record<K> a, const bucket_record<K>& b)
{
    return a.idx < b.idx;
}

template <class InputStream, class K>
uint64_t packed_read(InputStream& is, bucket_record<K>& record)
{
    using io::packed::read;
    std::size_t len;
    auto bytes = read(is, len) + read(is, record.idx);

    record.keys.resize(len);
    for (std::size_t i = 0; i < len; ++i)
        bytes += read(is, record.keys[i]);
    return bytes;
}

template <class OutputStream, class K>
uint64_t packed_write(OutputStream& os, const bucket_record<K>& record)
{
    using io::packed::write;
    auto bytes = write(os, record.keys.size()) + write(os, record.idx);
    for (const auto& key : record.keys)
        write(os, key);
    return bytes;
}

template <class K>
using chunk_iterator = util::chunk_iterator<bucket_record<K>>;

template <class K>
std::size_t hash(const K& key, uint64_t seed)
{
    using meta::hashing::hash_append;
    farm_hash_seeded hasher{seed};
    hash_append(hasher, key);
    return static_cast<std::size_t>(hasher);
}
}

template <class K>
perfect_hash_builder<K>::perfect_hash_builder(options opts)
    : opts_(opts), // parens to force bad compilers to locate cctor
      num_buckets_{opts.num_keys / opts.num_per_bucket + 1},
      num_chunks_{0}
{
    filesystem::make_directory(opts_.prefix);
    bucket_seed_ = std::random_device{}();

    buffer_.reserve(opts_.max_ram / sizeof(hashed_key));
}

template <class K>
void perfect_hash_builder<K>::flush_chunk()
{
    if (buffer_.empty())
        return;

    auto filename
        = opts_.prefix + "/chunk-" + std::to_string(num_chunks_) + ".bin";

    std::sort(buffer_.begin(), buffer_.end());
    std::ofstream output{filename, std::ios::binary};
    for (auto it = buffer_.begin(); it != buffer_.end();)
    {
        auto range = std::equal_range(it, buffer_.end(), *it);

        io::packed::write(output, static_cast<std::size_t>(std::distance(
                                      range.first, range.second)));
        io::packed::write(output, range.first->idx);
        for (; range.first != range.second; ++range.first)
            io::packed::write(output, range.first->key);

        it = range.second;
    }

    buffer_.clear();
    ++num_chunks_;
}

template <class K>
void perfect_hash_builder<K>::operator()(const K& key)
{
    if (buffer_.size() == buffer_.capacity())
        flush_chunk();
    buffer_.emplace_back(mph::hash(key, bucket_seed_) % num_buckets_, key);
}

template <class K>
void perfect_hash_builder<K>::write()
{
    if (!buffer_.empty())
        flush_chunk();

    // free the buffer memory
    std::vector<hashed_key>{}.swap(buffer_);

    merge_chunks_by_bucket_id();
    sort_buckets_by_size();
    merge_chunks_by_bucket_size();
    construct_perfect_hash();

    filesystem::delete_file(opts_.prefix + "/buckets.bin");
}

template <class K>
void perfect_hash_builder<K>::merge_chunks_by_bucket_id()
{
    {
        std::vector<mph::chunk_iterator<K>> iterators;
        for (uint64_t i = 0; i < num_chunks_; ++i)
            iterators.emplace_back(opts_.prefix + "/chunk-" + std::to_string(i)
                                   + ".bin");

        std::ofstream output{opts_.prefix + "/buckets.bin", std::ios::binary};
        util::multiway_merge(iterators.begin(), iterators.end(),
                             [&](mph::bucket_record<K>&& bucket)
                             {
                                 io::packed::write(output, bucket);
                             });
    }

    // delete temporary files
    for (uint64_t i = 0; i < num_chunks_; ++i)
        filesystem::delete_file(opts_.prefix + "/chunk-" + std::to_string(i)
                                + ".bin");
    num_chunks_ = 0;
}

template <class K>
void perfect_hash_builder<K>::sort_buckets_by_size()
{
    // compute the number of buffered keys we can have
    //
    // storage for this step is broken into two pieces: one huge vector for
    // holding keys that we've read out of the buckets file, and one
    // smaller vector that holds the starting and ending position for the
    // whole buckets that we've read from the buckets file
    //
    // the total RAM usage is approximately
    // num_buf_keys * sizeof(K)
    // + num_buckets / num_keys * num_buf_keys * sizeof(array_view)
    // which is solved to get the number of buffered keys we should have

    auto num_buf_keys = static_cast<std::size_t>(
        opts_.max_ram
        / (sizeof(K)
           + sizeof(util::array_view<K>) * static_cast<double>(num_buckets_)
                 / opts_.num_keys));

    auto num_buf_buckets = static_cast<std::size_t>(
        num_buf_keys * static_cast<double>(num_buckets_) / opts_.num_keys);

    std::vector<K> buffered_keys(num_buf_keys);
    std::vector<util::array_view<K>> buckets(num_buf_buckets);

    auto insert_it = buffered_keys.begin();
    auto bucket_it = buckets.begin();

    mph::chunk_iterator<K> it{opts_.prefix + "/buckets.bin"};
    printing::progress progress{" > Sorting buckets by size: ",
                                it.total_bytes()};
    for (; it != mph::chunk_iterator<K>{}; ++it)
    {
        progress(it.bytes_read());
        auto& bucket = *it;

        if (bucket.keys.size() > static_cast<std::size_t>(std::distance(
                                     insert_it, buffered_keys.end()))
            || bucket_it == buckets.end())
        {
            // we can't fit this bucket into our buffer, so we need to sort
            // and flush
            flush_bucket_chunk(buckets.begin(), bucket_it);

            insert_it = buffered_keys.begin();
            bucket_it = buckets.begin();
        }

        auto bucket_end
            = std::move(bucket.keys.begin(), bucket.keys.end(), insert_it);
        *bucket_it++ = util::array_view<K>{&*insert_it, bucket.keys.size()};
        insert_it = bucket_end;
    }

    if (bucket_it != buckets.begin())
        flush_bucket_chunk(buckets.begin(), bucket_it);
}

template <class K>
template <class Iterator>
void perfect_hash_builder<K>::flush_bucket_chunk(Iterator begin, Iterator end)
{
    std::sort(begin, end,
              [](const util::array_view<K>& a, const util::array_view<K>& b)
              {
                  return a.size() > b.size();
              });

    std::ofstream chunk{opts_.prefix + "/chunk-" + std::to_string(num_chunks_)
                            + ".bin",
                        std::ios::binary};
    std::for_each(begin, end, [&](const util::array_view<K>& bucket)
                  {
                      io::packed::write(chunk, bucket.size());
                      io::packed::write(chunk,
                                        mph::hash(bucket[0], bucket_seed_)
                                            % num_buckets_);
                      for (const auto& key : bucket)
                          io::packed::write(chunk, key);
                  });
    ++num_chunks_;
}

template <class K>
void perfect_hash_builder<K>::merge_chunks_by_bucket_size()
{
    std::vector<mph::chunk_iterator<K>> iterators;
    for (uint64_t i = 0; i < num_chunks_; ++i)
    {
        iterators.emplace_back(opts_.prefix + "/chunk-" + std::to_string(i)
                               + ".bin");
    }

    std::ofstream output{opts_.prefix + "/buckets.bin", std::ios::binary};
    util::multiway_merge(
        iterators.begin(), iterators.end(),
        // sort records at head of chunks by their size
        // (descending) rather than their bucket index
        [](const mph::bucket_record<K>& a, const mph::bucket_record<K>& b)
        {
            return a.keys.size() > b.keys.size();
        },
        // never merge two records together
        [](const mph::bucket_record<K>&, const mph::bucket_record<K>&)
        {
            return false;
        },
        [&](mph::bucket_record<K>&& bucket)
        {
            io::packed::write(output, bucket);
        });

    // delete temporary files
    for (uint64_t i = 0; i < num_chunks_; ++i)
    {
        filesystem::delete_file(opts_.prefix + "/chunk-" + std::to_string(i)
                                + ".bin");
    }
    num_chunks_ = 0;
}

namespace mph
{
template <class K>
std::vector<std::size_t> hashes_for_bucket(const mph::bucket_record<K>& bucket,
                                           std::size_t seed)
{
    std::vector<std::size_t> hashes(bucket.keys.size());
    std::transform(bucket.keys.begin(), bucket.keys.end(), hashes.begin(),
                   [&](const K& key)
                   {
                       return mph::hash(key, seed);
                   });
    std::sort(hashes.begin(), hashes.end());
    if (std::adjacent_find(hashes.begin(), hashes.end()) != hashes.end())
        throw std::runtime_error{"hash collision within bucket"};
    return hashes;
}

template <class ForwardIterator, class OutputIterator>
void hashes_to_indices(ForwardIterator begin, ForwardIterator end,
                       OutputIterator output, std::size_t seed, std::size_t mod)
{
    std::transform(begin, end, output, [&](const std::size_t& key)
                   {
                       return farm::hash_len_16(key, seed) % mod;
                   });
}

inline bool insert_bucket(std::vector<std::size_t>& indices,
                          std::vector<bool>& occupied_slots, std::size_t idx,
                          uint16_t seed, util::disk_vector<uint16_t>& seeds)
{
    auto iit = indices.begin();
    for (; iit != indices.end(); ++iit)
    {
        if (occupied_slots[*iit])
            break;
        occupied_slots[*iit] = true;
    }

    // if we failed to place everything without collisions, unset
    // the bits and try the next seed
    if (iit != indices.end())
    {
        for (auto iit2 = indices.begin(); iit2 != iit; ++iit2)
            occupied_slots[*iit2] = false;
        return false;
    }
    // otherwise, this seed worked, so store it and move on
    else
    {
        seeds[idx] = seed;
        return true;
    }
}
}

template <class K>
void perfect_hash_builder<K>::construct_perfect_hash()
{
    auto num_bins = static_cast<std::size_t>(
        std::ceil(opts_.num_keys / opts_.load_factor));
    std::vector<bool> occupied_slots(num_bins, false);

    {
        util::disk_vector<uint16_t> seeds{opts_.prefix + "/seeds.tmp.bin",
                                          num_buckets_};

        {
            mph::chunk_iterator<K> it{opts_.prefix + "/buckets.bin"};
            printing::progress progress{" > Constructing hash: ",
                                        it.total_bytes()};
            for (; it != mph::chunk_iterator<K>{}; ++it)
            {
                progress(it.bytes_read());
                const auto& bucket = *it;

                auto hashes = mph::hashes_for_bucket(bucket, bucket_seed_);

                std::vector<std::size_t> indices(bucket.keys.size());
                bool success = false;
                const uint16_t max_probes
                    = std::numeric_limits<uint16_t>::max();
                for (uint16_t i = 0; i < max_probes && !success; ++i)
                {
                    auto seed = static_cast<std::size_t>(i);

                    mph::hashes_to_indices(hashes.begin(), hashes.end(),
                                           indices.begin(), seed, num_bins);

                    success = mph::insert_bucket(indices, occupied_slots,
                                                 bucket.idx, i, seeds);
                }
                if (!success)
                    throw std::runtime_error{
                        "could not find a seed for a bucket in "
                        "minimal perfect hash generation"};
            }
        }

        LOG(progress) << "> Compressing seeds...\n" << ENDLG;

        // compress the seed vector
        succinct::make_compressed_vector(opts_.prefix + "/seeds", seeds.begin(),
                                         seeds.end());
    }

    filesystem::remove_all(opts_.prefix + "/seeds.tmp.bin");

    LOG(progress) << "> Minimizing hash...\n" << ENDLG;

    // minify the hash using a succinct::sarray + sarray_rank to compress
    // the range via rank() queries
    std::vector<uint64_t> positions;
    positions.reserve(occupied_slots.size() - opts_.num_keys);
    for (std::size_t i = 0; i < occupied_slots.size(); ++i)
    {
        if (!occupied_slots[i])
            positions.push_back(i);
    }
    std::vector<bool>{}.swap(occupied_slots);
    auto storage = succinct::make_sarray(
        opts_.prefix + "/sarray", positions.begin(), positions.end(), num_bins);
    succinct::sarray_rank{opts_.prefix + "/sarray", storage};

    std::ofstream metadata{opts_.prefix + "/hash-metadata.bin",
                           std::ios::binary};
    io::packed::write(metadata, bucket_seed_);
    io::packed::write(metadata, num_bins);

    LOG(progress) << "> Minimum perfect hash constructed\n" << ENDLG;
}
}
}
