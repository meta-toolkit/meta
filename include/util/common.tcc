/**
 * @file common.tcc
 */

#include <sstream>

namespace meta {

template <class T>
std::string common::toString(const T & value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

}
