/**
 * @file mph_vocab.cpp
 * @author Chase Geigle
 */

#include "meta/hashing/perfect_hash.h"
#include "meta/hashing/perfect_hash_builder.h"

int main(int argc, char** argv)
{
    using namespace meta;

    logging::set_cerr_logging();

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " file.txt" << std::endl;
        return 1;
    }

    using mph_builder = hashing::perfect_hash_builder<std::string>;
    using options_type = mph_builder::options;

    options_type options;
    options.prefix = "hashed-vocab";
    options.num_keys = filesystem::num_lines(argv[1]);

    mph_builder builder{options};

    {
        std::ifstream input{argv[1]};
        std::string line;

        while (std::getline(input, line))
            builder(line);
    }

    builder.write();

    hashing::perfect_hash<std::string> mph{"hashed-vocab"};
    std::ifstream input{argv[1]};
    std::string line;

    std::vector<std::string> vocab(options.num_keys);
    while (std::getline(input, line))
    {
        auto id = mph(line);
        if (!vocab[id].empty())
        {
            std::cerr << "Collision: " << line << " and " << vocab[id]
                      << std::endl;
            return 1;
        }
        vocab[id] = line;
        std::cout << line << " -> " << id << "\n";
    }

    for (std::size_t id = 0; id < vocab.size(); ++id)
    {
        if (vocab[id].empty())
        {
            std::cerr << "Unused term id: " << id << std::endl;
            return 1;
        }
    }

    return 0;
}
