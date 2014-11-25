#include <iostream>

#include "logging/logger.h"
#include "parser/io/ptb_reader.h"
#include "parser/trees/visitors/annotation_remover.h"
#include "parser/trees/visitors/empty_remover.h"
#include "parser/trees/visitors/unary_chain_remover.h"
#include "parser/trees/visitors/multi_transformer.h"

int main(int argc, char** argv)
{
    using namespace meta;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " trees.mrg" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    parser::multi_transformer<parser::annotation_remover, parser::empty_remover,
                              parser::unary_chain_remover> transformer;

    for (auto& tree : parser::io::extract_trees(argv[1]))
    {
        parser::parse_tree t{std::move(tree)};
        std::cout << "Original: " << std::endl;
        std::cout << t << std::endl;

        std::cout << "Transformed: " << std::endl;
        t.transform(transformer);
        std::cout << t << std::endl;
    }
    return 0;
}
