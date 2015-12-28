/**
 * @file forward_to_libsvm.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include "meta/index/forward_index.h"
#include "meta/logging/logger.h"
#include "meta/util/progress.h"

using namespace meta;

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage:\t" << argv[0] << " config.toml output-file"
                  << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto idx = index::make_index<index::forward_index>(*config);
    {
        std::ofstream output{argv[2]};
        printing::progress progress{" > Converting to libsvm: ",
                                    idx->num_docs()};
        for (const auto& did : idx->docs())
        {
            progress(did);
            output << idx->liblinear_data(did) << "\n";
        }
    }

    return 0;
}
