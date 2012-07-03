/**
 * @file common.cpp
 */

template <class T>
string Common::toString(const T & value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}
