/**
 * @file build_mph_lm.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "meta/lm/mph_language_model.h"
#include "meta/logging/logger.h"

int main(int argc, char** argv)
{
    using namespace meta;

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    try
    {
        lm::mph_language_model model{*config};
    }
    catch (const std::exception& ex)
    {
        std::cerr << "hash seed: " << hashing::detail::get_process_seed()
                  << std::endl;
        throw;
    }
    return 0;
}
