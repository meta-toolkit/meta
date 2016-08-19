/**
 * @file feature-summary.cpp
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "cpptoml.h"
#include "meta/classify/multiclass_dataset.h"
#include "meta/classify/multiclass_dataset_view.h"
#include "meta/features/all.h"
#include "meta/features/selector_factory.h"
#include "meta/index/forward_index.h"
#include "meta/logging/logger.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/util/shim.h"
#include <iostream>

using namespace meta;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    // Register additional analyzers
    parser::register_analyzers();
    sequence::register_analyzers();

    auto config = cpptoml::parse_file(argv[1]);
    auto feature_config = config->get_table("features");
    if (!feature_config)
    {
        std::cerr << "Missing [features] config table" << std::endl;
        return 1;
    }

    auto f_idx = index::make_index<index::memory_forward_index>(*config);

    classify::multiclass_dataset dset{f_idx};
    classify::multiclass_dataset_view dset_vw(dset);

    auto selector = features::make_selector(*config, dset_vw);

    selector->select(100);
    selector->print_summary(f_idx, 10);

    return 0;
}
