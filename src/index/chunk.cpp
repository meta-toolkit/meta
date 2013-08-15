/**
 * @file chunk.cpp
 * @author Sean Massung
 */

#include "index/chunk.h"

namespace meta {
namespace index {

chunk::chunk(const std::string & path, uint32_t size):
    _path(path), _size(size)
{ /* nothing */ }

bool chunk::operator<(const chunk & other) const
{
    return size() < other.size();
}

std::string chunk::path() const
{
    return _path;
}

uint32_t chunk::size() const
{
    return _size;
}

}
}
