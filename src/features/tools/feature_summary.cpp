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
#include "features/make_feature_selector.h"
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
    auto feature_config = config.get_group("features");
    if (!feature_config)
    {
        std::cerr << "Missing [features] config group" << std::endl;
        return 1;
    }

    auto f_idx = index::make_index<index::memory_forward_index>(argv[1]);
    auto selector
        = features::make_selector<features::information_gain>(argv[1], f_idx);
    // auto selector = features::make_selector<features::chi_square>{argv[1],
    // f_idx};
    selector->select(100);
    selector->print_summary(10);
}
