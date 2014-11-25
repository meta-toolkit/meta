#include <iostream>

#include "logging/logger.h"
#include "parser/io/ptb_reader.h"
#include "parser/trees/visitors/annotation_remover.h"
#include "parser/trees/visitors/empty_remover.h"
#include "parser/trees/visitors/unary_chain_remover.h"

int main(int argc, char** argv)
{
    using namespace meta;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " trees.mrg" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    parser::annotation_remover ann_rem;
    parser::empty_remover empty_rem;
    parser::unary_chain_remover uchain_rem;
    for (auto& tree : parser::io::extract_trees(argv[1]))
    {
        parser::parse_tree t{std::move(tree)};
        std::cout << "Original: " << std::endl;
        std::cout << t << std::endl;

        std::cout << "Transformed: " << std::endl;
        t.transform(ann_rem);
        t.transform(empty_rem);
        t.transform(uchain_rem);
        std::cout << t << std::endl;
    }
    return 0;
}
