/**
 * @file common.h
 * Includes code shared by many classes.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <chrono>
#include <iostream>
#include <unordered_map>
#include <string>
#include "util/invertible_map.h"
#include "cpptoml.h"

namespace meta {

/**
 * Functions used by many classes is contained here.
 */
namespace common
{
    /**
     *
     */
    template <class T>
    inline void write_binary(std::ostream & out, const T & elem)
    {
        out.write(reinterpret_cast<const char*>(&elem), sizeof(T));
    }

    inline void write_binary(std::ostream & out, const std::string & str)
    {
        out.write(str.c_str(), str.size() + 1);
    }

    /**
     *
     */
    template <class T>
    inline void read_binary(std::istream & in, T & elem)
    {
        in.read(reinterpret_cast<char*>(&elem), sizeof(T));
    }

    inline void read_binary(std::istream & in, std::string & str) {
        std::getline(in, str, '\0');
    }

    /**
     * Converts and type to a string representation.
     * @param value - the object to convert
     * @return the string representation of vaue
     */
    template <class T>
    std::string to_string(const T & value);

    /**
     * @param number
     * @return the parameter with commas added every thousandths place
     */
    inline std::string add_commas(const std::string & number);

    /**
     * Calculates a file's size in bytes with support for files over 4GB.
     * @param filename The path for the file
     * @return the number of bytes in the file
     */
    inline uint64_t file_size(const std::string & filename);

    /**
     * @param filename
     * @return true if the file exists
     */
    inline bool file_exists(const std::string & filename);

    /**
     * @param filename
     * @return the number of newline characters in the paramter
     */
    inline uint64_t num_lines(const std::string & filename);

    /**
     * @param str
     * @return the parameter string colored green
     */
    inline std::string make_green(std::string str)
    {
        return "\033[32m" + str + "\033[0m";
    }

    /**
     * @param str
     * @return the parameter string colored red
     */
    inline std::string make_red(std::string str)
    {
        return "\033[31m" + str + "\033[0m";
    }

    /**
     * @param str
     * @return the parameter string bolded
     */
    inline std::string make_bold(std::string str)
    {
        return "\033[1m" + str + "\033[22m";
    }

    /**
     * Converts a number of bytes into a human-readable number.
     * @param num_bytes
     * @return a human-readable string
     */
    inline std::string bytes_to_units(double num_bytes);

    /**
     * Times a given function.
     * @param functor the function to be timed
     * @return the length of time, expressed as a Duration, the function
     *  took to run. Defaults to milliseconds.
     */
    template <class Duration = std::chrono::milliseconds, class Functor>
    Duration time(Functor && functor);

    /**
     * Starts output from a call to show_progess by displaying 0% completion.
     * This is an optional function, and may be useful if the first call to
     * show_progress doesn't start exactly at 0%.
     * @param prefix The text to show before the percentage
     */
    inline void start_progress(const std::string & prefix);

    /**
     * @param idx The current progress in the operation
     * @param max The maximum value of idx, when it is done
     * @param freq How often to write output to the terminal
     * @param prefix The text to show before the percentage
     */
    inline void show_progress(size_t idx, size_t max, size_t freq, const std::string & prefix = "");

    /**
     * Ends output from a call to show_progess by displaying 100% completion.
     * @param prefix The text to show before the percentage
     */
    inline void end_progress(const std::string & prefix);

    /**
     * This safe_at allows the use of a hash function to be specified.
     * @return the value for a given key in map if it exists; otherwise,
     * the default Value() is returned.
     */
    template <class Key,
              class Value,
              class... Args,
              template <class, class, class...> class Map> 
    Value safe_at(const Map<Key, Value, Args...> & map, const Key & key);

    /**
     * Memoizes a std::function.
     * @param fun the std::function to be memoized
     */
    template <class Result, class... Args>
    std::function<Result(Args...)> memoize(std::function<Result(Args...)> fun);

    /**
     * Saves any arbitrary mapping to the disk.
     * @param map The map to read key, value pairs from
     * @param filename The name to save the mapping as
     */
    template <class Key, class Value>
    void save_mapping(const util::invertible_map<Key, Value> & map,
                      const std::string & filename);

    /**
     * Vector-specific version of save_mapping: saves any arbitrary mapping
     * to the disk.
     * @param map The map to read key, value pairs from
     * @param filename The name to save the mapping as
     */
    template <class T>
    void save_mapping(const std::vector<T> & vec,
                      const std::string & filename);

    /**
     * @param map The map to load information into
     * @param filename The file containing key, value pairs
     */
    template <class Key, class Value>
    void load_mapping(util::invertible_map<Key, Value> & map,
                      const std::string & filename);

    /**
     * Vector-specific version of load_mapping.
     * @param vec The vector to load information into
     * @param filename The file containing the vector's data.
     */
    template <class T>
    void load_mapping(std::vector<T> & vec, const std::string & filename);

    /**
     * Constructs a unique ptr in place.
     * @param args The parameters to the constructor
     */
    template <class T, class... Args>
    std::unique_ptr<T> make_unique(Args &&... args);
}

}

#include "util/common.tcc"
#endif
