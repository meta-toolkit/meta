#include <iostream>

#include "logging/logger.h"
#include "parser/io/ptb_reader.h"
#include "parser/trees/transformers/annotation_remover.h"

int main(int argc, char** argv)
{
    using namespace meta;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " trees.mrg" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    parser::annotation_remover trns;
    for (auto& tree : parser::io::extract_trees(argv[1]))
    {
        parser::parse_tree t{std::move(tree)};
        std::cout << "Original: " << std::endl;
        std::cout << t << std::endl;

        std::cout << "Transformed: " << std::endl;
        t.transform(trns);
        std::cout << t << std::endl;
    }
    return 0;
}
