/**
 * @file index.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "index/inverted_index.h"
#include "caching/all.h"
#include "logging/logger.h"
#include "parser/analyzers/tree_analyzer.h"
#include "sequence/analyzers/ngram_pos_analyzer.h"
#include "util/time.h"

using namespace meta;

/**
 * Simple demo app to create an index, or load one that's already been created.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    // Turn on logging to std::cerr.
    logging::set_cerr_logging();

    // Register additional analyzers
    parser::register_analyzers();
    sequence::register_analyzers();

    // Time how long it takes to create the index. By default, common::time's
    //  unit of measurement is milliseconds.
    auto time = common::time([&]()
    {
        // Creates an inverted index with no cache. We don't need a cache here
        //  since we're never searching the index, only building it.
        auto idx = index::make_index<index::inverted_index>(argv[1]);

        // Print out some data about the corpus.
        std::cout << "Number of documents: " << idx->num_docs() << std::endl;
        std::cout << "Avg Doc Length: " << idx->avg_doc_length() << std::endl;
        std::cout << "Unique Terms: " << idx->unique_terms() << std::endl;
    });

    std::cout << "Index generation took: " << time.count() / 1000.0
              << " seconds" << std::endl;

    return 0;
}
