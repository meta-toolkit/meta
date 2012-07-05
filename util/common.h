/**
 * @file common.h
 * Includes code shared by many classes.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <sstream>

using std::string;
using std::stringstream;

/**
 * Functions used by many classes is contained here.
 */
namespace Common
{
    template <class T>
    string toString(const T & value);

    inline string makeGreen(string str)
    {
        return "\033[1;32m" + str + "\033[0m";
    }

    inline string makeRed(string str)
    {
        return "\033[1;31m" + str + "\033[0m";
    }
}

#include "common.cpp"
#endif
