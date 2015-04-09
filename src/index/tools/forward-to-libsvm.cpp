/**
 * @file forward-to-libsvm.cpp
 * @author Chase Geigle
 */

#include "index/forward_index.h"

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

    auto idx = index::make_index<index::forward_index>(argv[1]);
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
