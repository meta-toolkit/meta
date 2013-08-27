/**
 * @file config_reader.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include "tokenizers/tokenizer.h"
#include "cpptoml.h"

namespace meta {
namespace io {

/**
 * A very simple configuration file reader.
 */
namespace config_reader
{
    /**
     * @param path The path to the config file
     * @return a mapping of option name to option value
     */
    cpptoml::toml_group read(const std::string & path);
}

}
}

#endif
