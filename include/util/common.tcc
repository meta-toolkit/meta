/**
 * @file common.tcc
 */

#include <sstream>
#include "util/common.h"

namespace meta {
namespace common {

template <class T>
std::string to_string(const T & value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string bytes_to_units(uint64_t num_bytes)
{
    std::string units = "bytes";
    for(auto & u: {"KB", "MB", "GB", "TB"})
    {
        if(num_bytes >= 1024)
        {
            num_bytes /= 1024;
            units = u;
        }
    }

    return to_string(num_bytes) + " " + units;
}

template <class Duration, class Functor>
Duration time(Functor && functor) {
    auto start = std::chrono::steady_clock::now();
    functor();
    auto end   = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<Duration>(end - start);
}

void show_progress(size_t idx, size_t max, size_t freq, const std::string & prefix)
{
    if(idx % freq == 0)
    {
        std::cerr << prefix << static_cast<double>(idx) / max * 100 << "%    \r";
        std::flush(std::cerr);
    }
}

void end_progress(const std::string & prefix)
{
    std::cerr << prefix << "100%         " << std::endl;
}

template <class Key, class Value, class Hash>
Value safe_at(const std::unordered_map<Key, Value, Hash> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Value{};
    return it->second;
}

template <class Key, class Value>
Value safe_at(const std::unordered_map<Key, Value> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Value{};
    return it->second;
}

}
}
