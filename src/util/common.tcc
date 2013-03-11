/**
 * @file common.tcc
 */

#include <sstream>

template <class T>
std::string Common::toString(const T & value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}
