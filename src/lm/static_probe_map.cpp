/**
 * @file static_probe_map.cpp
 * @author Sean Massung
 */

#include "lm/static_probe_map.h"

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
    auto hashed = hash_(key);
    auto idx = (hashed % (table_.size() / 2)) * 2;

    while (true)
    {
        if (table_[idx] == uint64_t{0})
        {
            table_[idx] = hashed;
            // pack prob and float into uint64_t slot next to key val
            uint64_t& ref = table_[idx + 1];
            *reinterpret_cast<float*>(&ref) = prob;
            *(reinterpret_cast<float*>(&ref) + 1) = backoff;
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
    auto hashed = hash_(key);
    auto idx = (hashed % (table_.size() / 2)) * 2;

    while (true)
    {
        if (table_[idx] == uint64_t{0})
            return util::nullopt;

        if (table_[idx] == hashed)
            return util::optional<lm_node>{lm_node{table_[idx + 1]}};

        idx = (idx + 2) % table_.size();
    }
}
}
}
