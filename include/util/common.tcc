/**
 * @file common.tcc
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <map>
#include <sstream>
#include <sys/stat.h>
#include "util/common.h"
#include "io/mmap_file.h"

namespace meta {
namespace common {

template <class T>
std::string to_string(const T & value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string add_commas(const std::string & number)
{
    std::string ret{""};
    size_t counter = 0;
    for(auto it = number.rbegin(); it != number.rend(); ++it, ++counter)
    {
        if(counter != 0 && counter != number.size() && counter % 3 == 0)
            ret = ',' + ret;
        ret = *it + ret;
    }

    return ret;
}

uint64_t file_size(const std::string & filename)
{
    if(!file_exists(filename))
        return 0;

    struct stat64 st;
    stat64(filename.c_str(), &st);
    return st.st_size;
}

bool file_exists(const std::string & filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    if(f != nullptr)
    {
        fclose(f);
        return true;
    }
    return false;
}

uint64_t num_lines(const std::string & filename)
{
    io::mmap_file file{filename};
    uint64_t num = 0;

    std::string progress = " > Counting lines in file ";
    for(uint64_t idx = 0; idx < file.size(); ++idx)
    {
        common::show_progress(idx, file.size(), 32 * 1024 * 1024, progress);
        if(file.start()[idx] == '\n')
            ++num;
    }
    common::end_progress(progress);

    return num;
}

std::string bytes_to_units(double num_bytes)
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

    num_bytes = static_cast<double>(static_cast<int>(num_bytes * 100)) / 100;
    return to_string(num_bytes) + " " + units;
}

template <class Duration, class Functor>
Duration time(Functor && functor)
{
    auto start = std::chrono::steady_clock::now();
    functor();
    auto end   = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<Duration>(end - start);
}

void start_progress(const std::string & prefix)
{
    std::cerr << prefix << "0%\r";
    std::flush(std::cerr);
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

template <class Key,
          class Value,
          class... Args,
          template <class, class, class...> class Map> 
Value safe_at(const Map<Key, Value, Args...> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Value{};
    return it->second;
}

template <class Result, class... Args>
std::function<Result(Args...)> memoize(std::function<Result(Args...)> fun)
{
    return [fun](Args... args) {
        static std::map<std::tuple<Args...>, Result> map_;
        auto it = map_.find(std::make_tuple(args...));
        if(it != map_.end())
            return it->second;
        return map_[std::make_tuple(args...)] = fun(args...);
    };
}

template <class Key, class Value>
void save_mapping(const util::invertible_map<Key, Value> & map,
                  const std::string & filename)
{
    std::ofstream outfile{filename};
    for(auto & p: map)
        outfile << p.first << " " << p.second << "\n";
}

template <class T>
void save_mapping(const std::vector<T> & vec, const std::string & filename)
{
    std::ofstream outfile{filename};
    for(auto & v: vec)
        outfile << v << "\n";
}

template <class Key, class Value>
void load_mapping(util::invertible_map<Key, Value> & map,
                  const std::string & filename)
{
    std::ifstream input{filename};
    Key k;
    Value v;
    while((input >> k) && (input >> v))
        map.insert(std::make_pair(k, v));
}

template <class T>
void load_mapping(std::vector<T> & vec, const std::string & filename)
{
    std::ifstream input{filename};
    uint64_t size = common::num_lines(filename);
    vec.reserve(size);

    T val;
    while(input >> val)
        vec.push_back(val);
}

template <class T, class... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>{new T{std::forward<Args>(args)...}};
}

}
}
