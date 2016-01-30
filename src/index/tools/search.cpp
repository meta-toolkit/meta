/**
 * @file search.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <string>
#include <vector>
#include "meta/analyzers/analyzer.h"
#include "meta/caching/all.h"
#include "meta/corpus/document.h"
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/util/time.h"

using namespace meta;

/**
 * Demo app to load (or create) an index, and query some documents that already
 * exist in it. We create the query documents by setting each document's path.
 * For this demo, we need the corpus type to be "file-corpus" so the documents
 * can be recreated.
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

    // Register additional analyzers.
    parser::register_analyzers();
    sequence::register_analyzers();

    // Create an inverted index based on the config file.
    auto config = cpptoml::parse_file(argv[1]);
    auto idx = index::make_index<index::inverted_index>(*config);


    // Create a ranking class based on the config file.
    auto group = config->get_table("ranker");
    if (!group)
        throw std::runtime_error{"\"ranker\" group needed in config file!"};
    auto ranker = index::make_ranker(*group);

    // Use UTF-8 for the default encoding unless otherwise specified.
    auto encoding = config->get_as<std::string>("encoding").value_or("utf-8");

    // Time how long it takes to create the index. By default, common::time's
    //  unit of measurement is milliseconds.
    auto elapsed = common::time(
        [&]()
        {
            // Get a std::vector of doc_ids that have been indexed.
            auto docs = idx->docs();

            // Search for up to the first 20 documents; we hope that the first
            //  result is the original document itself since we're querying with
            //  documents that are already indexed.
            for (size_t i = 0; i < 20 && i < idx->num_docs(); ++i)
            {
                auto path = idx->doc_path(docs[i]);
                // Create a document and specify its path; its content will be
                //  filled by the analyzer.
                corpus::document query{doc_id{docs[i]}};
                query.content(filesystem::file_text(path), encoding);

                std::cout << "Ranking query " << (i + 1) << ": " << path
                          << std::endl;

                // Use the ranker to score the query over the index. By default,
                // the
                //  ranker returns 10 documents, so we will display the "top 10
                //  of
                //  10" docs.
                auto ranking = ranker->score(*idx, query);
                std::cout << "Showing top 10 results." << std::endl;

                uint64_t result_num = 1;
                for (auto& result : ranking)
                {
                    std::cout << result_num << ". "
                              << idx->doc_name(result.d_id) << " "
                              << result.score << std::endl;
                    if (result_num++ == 10)
                        break;
                }

                std::cout << std::endl;
            }
        });

    std::cout << "Elapsed time: " << elapsed.count() / 1000.0 << " seconds"
              << std::endl;

    return 0;
}
