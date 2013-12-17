/**
 * @file index.cpp
 */

#include <iostream>
#include <fstream>
#include "util/common.h"
#include "index/inverted_index.h"
#include "caching/all.h"
#include "logging/logger.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    using namespace meta::logging;

    logging::add_sink({std::cerr, [](const logger::log_line & ll) {
        return ll.severity() == logger::severity_level::progress;
    }, [](const logger::log_line & ll) {
        return " " + ll.str();
    }});

    logging::add_sink({std::cerr, logger::severity_level::trace});
    std::ofstream logfile{"meta.log"};
    logging::add_sink({logfile, logger::severity_level::info});

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
