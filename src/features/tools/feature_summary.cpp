/**
 * @file feature-summary.cpp
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <iostream>
#include "cpptoml.h"
#include "util/shim.h"
#include "features/all.h"
#include "features/selector_factory.h"
#include "index/forward_index.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto feature_config = config.get_table("features");
    if (!feature_config)
    {
        std::cerr << "Missing [features] config table" << std::endl;
        return 1;
    }

    auto f_idx = index::make_index<index::memory_forward_index>(argv[1]);
    auto selector = features::make_selector(config, f_idx);
    selector->select(100);
    selector->print_summary(10);
}
