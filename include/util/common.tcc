/**
 * @file common.tcc
 */

#include <sstream>

namespace meta {

template <class T>
std::string common::to_string(const T & value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

void common::show_progress(size_t idx, size_t max, size_t freq, const std::string & prefix)
{
    if(idx % freq == 0)
    {
        std::cerr << prefix << static_cast<double>(idx) / max * 100 << "%    \r";
        std::flush(std::cerr);
    }
}

void common::end_progress(const std::string & prefix)
{
    std::cerr << prefix << "100%         " << std::endl;
}

cpptoml::toml_group common::read_config(const std::string & path)
{
    std::ifstream file{path};
    if(!file.is_open())
        throw "Error: config file couldn't be opened";
    cpptoml::parser p{file};
    return p.parse();
}

template <class Key, class Value, class Hash>
Value common::safe_at(const std::unordered_map<Key, Value, Hash> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Value{};
    return it->second;
}

template <class Key, class Value>
Value common::safe_at(const std::unordered_map<Key, Value> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Value{};
    return it->second;
}

}
