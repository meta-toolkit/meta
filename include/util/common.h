/**
 * @file common.h
 * Includes code shared by many classes.
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
    std::string toString(const T & value);

    /**
     * @param str
     * @return the parameter string colored green
     */
    inline std::string makeGreen(std::string str)
    {
        return "\033[1;32m" + str + "\033[0m";
    }

    /**
     * @param str
     * @return the parameter string colored red
     */
    inline std::string makeRed(std::string str)
    {
        return "\033[1;31m" + str + "\033[0m";
    }

    /**
     * @param str
     * @return the parameter string bolded
     */
    inline std::string makeBold(std::string str)
    {
        return "\033[1m" + str + "\033[0m";
    }

    /**
     * @param idx
     * @param max
     * @param freq
     * @param prefix
     */
    inline void show_progress(size_t idx, size_t max, size_t freq, const std::string & prefix = "")
    {
        if(idx % freq == 0)
        {
            std::cerr << prefix << static_cast<double>(idx) / max * 100 << "%    \r";
            std::flush(std::cerr);
        }
    }

    inline void end_progress(const std::string & prefix)
    {
        std::cerr << prefix << "100%         " << std::endl;
    }

    /**
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
