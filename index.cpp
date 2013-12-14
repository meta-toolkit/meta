/**
 * @file index.cpp
 */

#include <iostream>
#include <fstream>
#include "util/common.h"
#include "index/inverted_index.h"
#include "caching/all.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    auto time = common::time([&]() {
        auto idx = index::make_index<index::inverted_index,
                                     caching::splay_cache>(argv[1], uint32_t{10000});
        std::cout << "Number of documents: " << idx.num_docs() << std::endl;
        std::cout << "Average Doc Length: " << idx.avg_doc_length() << std::endl;
        std::cout << "Unique Terms: " << idx.unique_terms() << std::endl;
    });

    std::cout << "Index generation took: " << time.count() / 1000.0
        << " seconds" << std::endl;

    return 0;
}
