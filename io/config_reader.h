#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <unordered_map>
#include <string>

namespace ConfigReader
{
    std::unordered_map<std::string, std::string> read(const std::string & path);
}

#endif
