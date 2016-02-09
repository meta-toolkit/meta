/**
 * @file embedding_coocur.cpp
 * @author Chase Geigle
 *
 * This tool decompresses the MeTA vocabulary and coocurrence matrix files
 * to input that the original GloVe tool can read.
 *
 * (This is mainly for sanity checking.)
 */

#include "cpptoml.h"
#include "meta/embeddings/coocur_iterator.h"
#include "meta/io/binary.h"
#include "meta/logging/logger.h"
#include "meta/util/progress.h"

using namespace meta;

int main(int argc, char** argv)
{
    using namespace embeddings;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    // extract building parameters
    auto embed_cfg = config->get_table("embeddings");
    auto prefix = *embed_cfg->get_as<std::string>("prefix");

    {
        std::ifstream input{prefix + "/vocab.bin", std::ios::binary};
        std::ofstream output{"vocab-glove.txt"};
        auto size = io::packed::read<uint64_t>(input);

        printing::progress progress{" > Decompressing vocab: ",
            size};
        for (uint64_t tid = 0; tid < size; ++tid)
        {
            progress(tid);
            auto word = io::packed::read<std::string>(input);
            auto count = io::packed::read<uint64_t>(input);

            output << word << " " << count << "\n";
        }
    }

    {
        coocur_iterator iter{prefix + "/coocur.bin"};
        printing::progress progress{" > Decompressing coocurrence matrix: ",
                                    iter.total_bytes()};
        std::ofstream output{"coocur-glove.bin", std::ios::binary};
        for (; iter != coocur_iterator{}; ++iter)
        {
            progress(iter.bytes_read());
            auto record = *iter;
            io::write_binary(output, (int)record.target);
            io::write_binary(output, (int)record.context);
            io::write_binary(output, record.weight);
        }
    }
}
