/**
 * @file meta_to_word2vec.cpp
 * @author Matt Kelly
 *
 * This tool converts learned word embeddings stored in MeTA style into the
 * word2vec storage style. This allows the embeddings to be run against
 * word2vec's accuracy tools.
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "cpptoml.h"
#include "meta/embeddings/word_embeddings.h"
#include "meta/logging/logger.h"
#include "meta/util/progress.h"

using namespace meta;
using namespace embeddings;

// Save vectors in word2vec format.
void save_w2v_vectors(const bool binary_output, const std::string prefix,
                      word_embeddings learned_embeddings)
{
    auto vector_size = learned_embeddings.vector_size();
    auto vocab = learned_embeddings.vocab();
    printing::progress progress{" > Saving word2vec embeddings: ",
                                vocab.size() * vector_size};

    const std::string file_path = prefix + "/embeddings.w2v.bin";
    FILE* file = fopen(file_path.c_str(), "wb");

    fprintf(file, "%lld %lld\n", (long long)vocab.size(), (long long)vector_size);
    for (size_t i = 0; i < vocab.size(); ++i)
    {
        fprintf(file, "%s ", vocab[i].c_str());
        auto target_vector = learned_embeddings.at(vocab[i]).v;
        if (binary_output)
        {
            for (std::size_t j = 0; j < vector_size; ++j)
            {
                // Copying seemed to work here. assuming
                // there is a better way to do this but
                // couldn't quite get it.
                auto value = static_cast<float>(target_vector[j]);
                fwrite(&value, sizeof(float), 1, file);
                progress(i * vector_size + j);
            }
        }
        else
        {
            for (std::size_t j = 0; j < vector_size; ++j)
            {
                fprintf(file, "%lf ", target_vector[j]);
                progress(i * vector_size + j);
            }
        }

        fprintf(file, "\n");
    }

    fclose(file);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto embed_cfg = config->get_table("embeddings");
    if (!embed_cfg)
    {
        std::cerr << "Missing [embeddings] configuration in " << argv[1]
                  << std::endl;
        return 1;
    }

    auto sgns_cfg = embed_cfg->get_table("sgns");
    if (!sgns_cfg)
    {
        std::cerr << "Missing [embeddings.sgns] configuration in " << argv[1]
                  << std::endl;
        return 1;
    }

    auto prefix = embed_cfg->get_as<std::string>("prefix");
    if (!prefix)
        throw word_embeddings_exception{
            "missing prefix key in configuration file"};

    auto binary_output = sgns_cfg->get_as<bool>("binary");
    if (!prefix)
        throw word_embeddings_exception{
            "missing prefix key in configuration file"};

    save_w2v_vectors(*binary_output, *prefix, load_embeddings(*embed_cfg));
}
