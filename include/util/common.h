/**
 * @file common.h
 * Includes code shared by many classes.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <unordered_map>
#include <string>

namespace meta {

/**
 * Functions used by many classes is contained here.
 */
namespace common
{
    /**
     * Converts and type to a string representation.
     * @param value - the object to convert
     * @return the string representation of vaue
     */
    template <class T>
    std::string to_string(const T & value);

    /**
     * @param str
     * @return the parameter string colored green
     */
    inline std::string make_green(std::string str)
    {
        return "\033[1;32m" + str + "\033[0m";
    }

    /**
     * @param str
     * @return the parameter string colored red
     */
    inline std::string make_red(std::string str)
    {
        return "\033[1;31m" + str + "\033[0m";
    }

    /**
     * @param str
     * @return the parameter string bolded
     */
    inline std::string make_bold(std::string str)
    {
        return "\033[1m" + str + "\033[0m";
    }

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
    template <class Key, class Value, class Hash>
    Value safe_at(const std::unordered_map<Key, Value, Hash> & map, const Key & key);

    /**
     * @return the value for a given key in map if it exists; otherwise,
     * the default Value() is returned.
     */
    template <class Key, class Value>
    Value safe_at(const std::unordered_map<Key, Value> & map, const Key & key);
}

}

#include "util/common.tcc"
#endif
