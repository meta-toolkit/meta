/**
 * @file path-predict.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "graph/algorithm/path_predict.h"
#include "logging/logger.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();
    graph::algorithm::path_predict ppredict{argv[1]};
}
