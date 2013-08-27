/**
 * @file config_reader.cpp
 */

#include <fstream>
#include <iostream>
#include "io/config_reader.h"
#include "tokenizers/tokenizers.h"
#include "index/forward_index.h"

namespace meta {
namespace io {

cpptoml::toml_group config_reader::read(const std::string & path)
{
    std::ifstream file{ path };
    if( !file.is_open() )
        std::cerr << "I am an exception and I couldn't open your file!" << std::endl;
    cpptoml::parser p{ file };
    return p.parse();
}

}
}
