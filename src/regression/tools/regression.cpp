/**
 * @file regression.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "meta/logging/logger.h"
#include "meta/regression/regressor_factory.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/stats/running_stats.h"
#include "meta/util/printing.h"

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
    auto reg_config = config->get_table("regressor");
    if (!reg_config)
    {
        std::cerr << "Missing regressor configuration group in " << argv[1]
                  << std::endl;
        return 1;
    }

    auto f_idx = index::make_index<index::forward_index>(*config);
    // load the documents into a dataset
    regression::regression_dataset dataset{
        f_idx, [&](doc_id did)
        {
            return *f_idx->metadata(did).get<double>("response");
        }};

    auto results = regression::cross_validate(*reg_config, dataset, 5);

    std::cout << "Avg (stddev) of metrics\n";

    stats::running_stats mae;
    stats::running_stats med_ae;
    stats::running_stats mse;
    stats::running_stats r2;

    for (const auto& m : results)
    {
        mae.add(m.mean_absolute_error);
        med_ae.add(m.median_absolute_error);
        mse.add(m.mean_squared_error);
        r2.add(m.r2_score);
    }

    std::cout << printing::make_bold("MAE:\t") << mae.mean() << " ("
              << mae.stddev() << ")\n";

    std::cout << printing::make_bold("MedAE:\t") << med_ae.mean() << " ("
              << med_ae.stddev() << ")\n";

    std::cout << printing::make_bold("MSE:\t") << mse.mean() << " ("
              << mse.stddev() << ")\n";

    std::cout << printing::make_bold("R^2:\t") << r2.mean() << " ("
              << r2.stddev() << ")" << std::endl;

    return 0;
}
