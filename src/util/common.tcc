/**
 * @file common.tcc
 */

#include <sstream>

using std::string;
using std::stringstream;

template <class T>
string Common::toString(const T & value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}
