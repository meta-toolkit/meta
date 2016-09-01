/**
 * @file perfect_hash_map.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_HASHING_PERFECT_HASH_MAP_H_
#define META_HASHING_PERFECT_HASH_MAP_H_

#include <cstdint>

#include "meta/config.h"
#include "meta/hashing/perfect_hash.h"
#include "meta/hashing/perfect_hash_builder.h"
#include "meta/io/filesystem.h"
#include "meta/io/mmap_file.h"
#include "meta/io/moveable_stream.h"
#include "meta/io/packed.h"
#include "meta/util/optional.h"
#include "meta/util/progress.h"

namespace meta
{
namespace hashing
{

namespace detail
{
template <class Value, class FingerPrint = uint32_t>
struct hash_record
{
    using fingerprint_type = FingerPrint;
    using value_type = Value;
    FingerPrint id;
    Value value;
};

template <class Value, class FingerPrint = uint32_t>
struct hashed_value
{
    uint64_t idx;
    hash_record<Value, FingerPrint> record;

    void merge_with(hashed_value&&)
    {
        throw std::runtime_error{"can't merge detail::hashed_value"};
    }
};

template <class OutputStream, class Value, class FingerPrint>
uint64_t packed_write(OutputStream& os,
                      const hash_record<Value, FingerPrint>& r)
{
    using io::packed::write;
    return write(os, r.id) + write(os, r.value);
}

template <class InputStream, class Value, class FingerPrint>
uint64_t packed_read(InputStream& is, hash_record<Value, FingerPrint>& r)
{
    using io::packed::read;
    return read(is, r.id) + read(is, r.value);
}

template <class OutputStream, class Value, class FingerPrint>
uint64_t packed_write(OutputStream& os,
                      const hashed_value<Value, FingerPrint>& hv)
{
    using io::packed::write;
    return write(os, hv.idx) + write(os, hv.record);
}

template <class InputStream, class Value, class FingerPrint>
uint64_t packed_read(InputStream& is, hashed_value<Value, FingerPrint>& hv)
{
    using io::packed::read;
    return read(is, hv.idx) + read(is, hv.record);
}

template <class HashedValue>
using hv_chunk_iterator = util::chunk_iterator<HashedValue>;
}

/**
 * Builder class for constructing the on-disk representation for a minimal
 * perfect hashing based hash map.
 *
 * `KeyType` represents the key to be hashed, which requires it to have an
 * overloaded `hash_append`.
 *
 * `ValueType` represents the mapped value type, which will be written
 * directly in binary to the disk for efficiency.
 *
 * `FingerPrint` represents the type to contain the fingerprint values for
 * each of the records in the perfect hash map, for (probabilistic)
 * false positive detection.
 */
template <class KeyType, class ValueType, class FingerPrint = uint32_t>
class perfect_hash_map_builder
{
  public:
    using record_type = detail::hash_record<ValueType, FingerPrint>;
    using fingerprint_type = typename record_type::fingerprint_type;
    using hash_builder_type = hashing::perfect_hash_builder<KeyType>;
    using options_type = typename hash_builder_type::options;
    using fingerprint_hash = hashing::seeded_hash<hashing::farm_hash_seeded>;

    /**
     * @param options The options for building the hash funciton (see
     * perfect_hash_builder::options)
     */
    perfect_hash_map_builder(const options_type& options)
        : options_(options),
          output_{options.prefix + "/values.bin.tmp", std::ios::binary},
          hash_builder_{make_unique<hash_builder_type>(options)},
          fingerprint_{47}
    {
        // nothing
    }

    /**
     * Handles a key/value pair.
     */
    void operator()(const KeyType& key, const ValueType& value)
    {
        (*hash_builder_)(key);

        io::packed::write(output_.stream(), key);
        io::packed::write(output_.stream(), value);
    }

    /**
     * Finalizes writing the perfect hash map to disk. This may take a
     * long time, depending on how much data is being stored.
     */
    void write()
    {
        output_.stream().close();

        LOG(progress) << "> Building hash function...\n" << ENDLG;
        hash_builder_->write();
        hash_builder_ = nullptr;

        reorder_values();
    }

  private:
    using hashed_value = detail::hashed_value<ValueType>;

    void reorder_values()
    {
        // hash every (key, value) we recorded using the perfect hash
        // function we generated in the first pass, then do a multi-way
        // merge to put the values into the right order according to the
        // new hash function
        LOG(info) << "Loading hash..." << ENDLG;
        hashing::perfect_hash<KeyType> hash{options_.prefix};
        LOG(info) << "Hash loaded" << ENDLG;

        uint64_t num_chunks = 0;

        {
            std::vector<hashed_value> buffer;
            buffer.reserve(options_.max_ram / sizeof(hashed_value));

            printing::progress progress{
                " > Reordering values: ",
                filesystem::file_size(options_.prefix + "/values.bin.tmp")};

            std::ifstream input{options_.prefix + "/values.bin.tmp",
                                std::ios::binary};

            for (uint64_t bytes = 0; input.peek() != EOF;)
            {
                progress(bytes);

                KeyType key;
                hashed_value h_value;
                bytes += io::packed::read(input, key);
                bytes += io::packed::read(input, h_value.record.value);
                h_value.record.id
                    = static_cast<fingerprint_type>(fingerprint_(key));
                h_value.idx = hash(key);

                buffer.push_back(h_value);

                if (buffer.size() == options_.max_ram / sizeof(hashed_value))
                {
                    flush_chunk(buffer, num_chunks);
                    ++num_chunks;
                }
            }

            if (!buffer.empty())
            {
                flush_chunk(buffer, num_chunks);
                ++num_chunks;
            }
        }

        filesystem::remove_all(options_.prefix + "/values.bin.tmp");

        std::vector<detail::hv_chunk_iterator<hashed_value>> iterators;
        iterators.reserve(num_chunks);
        for (uint64_t i = 0; i < num_chunks; ++i)
            iterators.emplace_back(options_.prefix + "/value-chunk."
                                   + std::to_string(i));

        std::ofstream output{options_.prefix + "/values.bin", std::ios::binary};
        util::multiway_merge(
            iterators.begin(), iterators.end(),
            // sort records at head of chunks by decreasing hash id
            [](const hashed_value& a, const hashed_value& b) {
                return a.idx < b.idx;
            },
            // never merge two records together
            [](const hashed_value&, const hashed_value&) { return false; },
            [&](hashed_value&& hv) {
                output.write(reinterpret_cast<char*>(&hv.record),
                             sizeof(hv.record));
            });

        // delete temporary files
        for (uint64_t i = 0; i < num_chunks; ++i)
        {
            filesystem::delete_file(options_.prefix + "/value-chunk."
                                    + std::to_string(i));
        }
    }

    void flush_chunk(std::vector<hashed_value>& buffer, uint64_t chunk_num)
    {
        std::sort(buffer.begin(), buffer.end(),
                  [](const hashed_value& a, const hashed_value& b) {
                      return a.idx < b.idx;
                  });

        std::ofstream chunk{options_.prefix + "/value-chunk."
                            + std::to_string(chunk_num)};
        for (const auto& hval : buffer)
        {
            io::packed::write(chunk, hval.idx);
            io::packed::write(chunk, hval.record);
        }
        buffer.clear();
    }

    options_type options_;
    io::mofstream output_;
    std::unique_ptr<hash_builder_type> hash_builder_;
    fingerprint_hash fingerprint_;
};

/**
 * Represents an entry in a perfect_hash_map, containing its index into
 * the dense values array and its actual value.
 */
template <class Value>
struct indexed_value
{
    uint64_t idx;
    Value value;
};

/**
 * An immutable minimal perfect hashing based hash_map, read from disk.
 */
template <class Key, class Value, class FingerPrint = uint32_t>
class perfect_hash_map
{
  public:
    using fingerprint_type = FingerPrint;
    using record_type = detail::hash_record<Value, FingerPrint>;
    using fingerprint_hash = seeded_hash<hashing::farm_hash_seeded>;
    using index_and_value_type = indexed_value<Value>;

    /**
     * @param prefix The folder on disk that contains the perfect_hash_map
     */
    perfect_hash_map(const std::string& prefix)
        : hash_{prefix}, file_{prefix + "/values.bin"}, fingerprint_{47}
    {
        // nothing
    }

    perfect_hash_map(perfect_hash_map&&) = default;

    /**
     * @return the perfect hash function used by the map
     */
    const perfect_hash<Key>& hash() const
    {
        return hash_;
    }

    /**
     * @param key The key to look up
     * @return An optional indexed_value that, if non-empty, indicates the
     * index and the value associated with this key
     */
    util::optional<index_and_value_type> index_and_value(const Key& key) const
    {
        auto idx = index(key);
        if (!idx)
            return util::nullopt;

        auto record = reinterpret_cast<const record_type*>(
            file_.begin() + *idx * sizeof(record_type));

        return index_and_value_type{*idx, record->value};
    }

    /**
     * @param key The key to look up
     * @return The index of this key in the dense value array, if it exists
     */
    util::optional<uint64_t> index(const Key& key) const
    {
        auto idx = hash_(key);
        auto id = static_cast<fingerprint_type>(fingerprint_(key));

        auto record = reinterpret_cast<const record_type*>(
            file_.begin() + idx * sizeof(record_type));

        if (id == record->id)
            return idx;

        return util::nullopt;
    }

    /**
     * @param key The key to look up
     * @return The value of this key, if it exists
     */
    util::optional<Value> at(const Key& key) const
    {
        auto idx = hash_(key);
        auto id = static_cast<fingerprint_type>(fingerprint_(key));

        auto record = reinterpret_cast<const record_type*>(
            file_.begin() + idx * sizeof(record_type));

        if (id == record->id)
            return record->value;

        return util::nullopt;
    }

    /**
     * @param idx The index into the dense values array
     * @return the value at that index
     * @note The behavior is undefined for indexes not obtained via
     * index() or index_and_value()
     */
    const Value& operator[](uint64_t idx) const
    {
        auto record = reinterpret_cast<const record_type*>(
            file_.begin() + idx * sizeof(record_type));
        return record->value;
    }

  private:
    perfect_hash<Key> hash_;
    io::mmap_file file_;
    fingerprint_hash fingerprint_;
};
}
}
#endif
