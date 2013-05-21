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

    /**
     * @param config
     */
    std::string get_config_string(const cpptoml::toml_group & config);

    /**
     * @return a Tokenizer as specified by a config object
     */
    std::shared_ptr<tokenizers::tokenizer> create_tokenizer(
        const cpptoml::toml_group & config);

    /**
     * Basic exception for config_reader interactions.
     */
    class config_reader_exception: public std::exception
    {
        public:
            
            config_reader_exception(const std::string & error):
                _error(error) { /* nothing */ }

            const char* what () const throw ()
            {
                return _error.c_str();
            }
       
        private:
       
            std::string _error;
    };
}

}
}

#endif
