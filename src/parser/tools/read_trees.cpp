#include <iostream>

#include "logging/logger.h"
#include "parser/io/ptb_reader.h"
#include "parser/trees/visitors/annotation_remover.h"
#include "parser/trees/visitors/empty_remover.h"
#include "parser/trees/visitors/unary_chain_remover.h"
#include "parser/trees/visitors/multi_transformer.h"
#include "parser/trees/visitors/head_finder.h"

using namespace meta;

struct annotation_checker : public parser::const_visitor<bool>
{
    bool operator()(const parser::leaf_node&) override
    {
        return true;
    }

    bool operator()(const parser::internal_node& inode) override
    {
        if (!inode.head_constituent())
            return false;

        if (!inode.head_lexicon())
            return false;

        bool res = true;
        inode.each_child([&](const parser::node* child)
                         {
            res &= child->accept(*this);
        });
        return res;
    }
};

int main(int argc, char** argv)
{

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " trees.mrg [trees2.mrg...]" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    parser::multi_transformer<parser::annotation_remover, parser::empty_remover,
                              parser::unary_chain_remover> transformer;

    parser::head_finder hf;
    annotation_checker ann_check;

    for (int arg = 1; arg < argc; ++arg)
    {
        std::cout << "Parsing: " << argv[arg] << "..." << std::endl;
        for (auto& tree : parser::io::extract_trees(argv[arg]))
        {
            parser::parse_tree t{std::move(tree)};
            std::cout << "Original: " << std::endl;
            std::cout << t << std::endl;

            std::cout << "Transformed: " << std::endl;
            t.transform(transformer);
            t.visit(hf);
            if (!t.visit(ann_check))
                throw std::runtime_error{"Failed to fully annotate heads"};
            std::cout << t << std::endl;
        }
    }
    return 0;
}
