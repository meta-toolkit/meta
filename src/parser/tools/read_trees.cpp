#include <iostream>

#include "logging/logger.h"
#include "parser/io/ptb_reader.h"

int main(int argc, char** argv)
{
    using namespace meta;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " trees.mrg" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    for (const auto& tree : parser::io::extract_trees(argv[1]))
        std::cout << tree << std::endl;
    return 0;
}
