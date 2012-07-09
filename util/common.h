/**
 * @file common.h
 * Includes code shared by many classes.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>

/**
 * Functions used by many classes is contained here.
 */
namespace Common
{
    template <class T>
    std::string toString(const T & value);

    inline std::string makeGreen(std::string str)
    {
        return "\033[1;32m" + str + "\033[0m";
    }

    inline std::string makeRed(std::string str)
    {
        return "\033[1;31m" + str + "\033[0m";
    }
}

#include "common.cpp"
#endif
