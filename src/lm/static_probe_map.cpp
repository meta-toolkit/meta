/**
 * @file static_probe_map.cpp
 * @author Sean Massung
 */

#include <cstring>
#include "lm/static_probe_map.h"
#include "util/hash.h"

namespace meta
{
namespace lm
{
static_probe_map::static_probe_map(const std::string& filename,
                                   uint64_t num_elems)
    : table_{filename, static_cast<uint64_t>((num_elems / 0.7) * 2)}
// load factor of 0.7; x2 for keys and vals
{
}

void static_probe_map::insert(const std::string& key, float prob, float backoff)
{
    auto hashed = hash(key);
    auto idx = (hashed % (table_.size() / 2)) * 2;

    while (true)
    {
        if (table_[idx] == uint64_t{0})
        {
            table_[idx] = hashed;

            // pack prob and float into uint64_t slot next to key val
            uint64_t buf = 0;
            std::memcpy(&table_[idx + 1], &prob, sizeof(float));
            std::memcpy(&buf, &backoff, sizeof(float));
            table_[idx + 1] |= (buf << 32);
            return;
        }

        if (table_[idx] == hashed)
            throw static_probe_map_exception{
                "key already exists (or collision)"};

        idx = (idx + 2) % table_.size();
    }
}

util::optional<lm_node> static_probe_map::find(const std::string& key) const
{
    auto hashed = hash(key);
    auto idx = (hashed % (table_.size() / 2)) * 2;

    while (true)
    {
        if (table_[idx] == uint64_t{0})
            return util::nullopt;

        if (table_[idx] == hashed)
            return {table_[idx + 1]};

        idx = (idx + 2) % table_.size();
    }
}

uint64_t static_probe_map::hash(const std::string& str) const
{
    util::murmur_hash<> hasher{seed_};
    hasher(str.data(), str.length());
    return static_cast<std::size_t>(hasher);
}
}
}
