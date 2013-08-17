/**
 * @file search.cpp
 */

#include <chrono>
#include <vector>
#include <string>
#include <iostream>

#include "io/config_reader.h"
#include "tokenizers/tokenizer.h"
#include "index/document.h"
#include "index/inverted_index.h"
#include "index/okapi_bm25.h"

using namespace meta;
using std::cerr;
using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    auto config = io::config_reader::read(argv[1]);
    std::string prefix = *cpptoml::get_as<std::string>(config, "prefix")
        + *cpptoml::get_as<std::string>(config, "dataset");
    std::string corpus_file = prefix 
        + "/" 
        + *cpptoml::get_as<std::string>(config, "list")
        + "-full-corpus.txt";

    std::vector<index::document> docs = index::document::load_docs(corpus_file, prefix);
    std::shared_ptr<tokenizers::tokenizer> tok = io::config_reader::create_tokenizer(config);

    //index::inverted_index idx{"my-index"};
    index::inverted_index idx{"my-index", docs, tok, argv[1]};
    index::okapi_bm25 ranker;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    for(size_t i = 0; i < 100; ++i)
    {
        index::document query{docs[i]};
        std::vector<std::pair<doc_id, double>> ranking = ranker.score(idx, query);
        cout << "Query " << i << ": " << query.path() << endl;

        for(size_t i = 0; i < ranking.size() && i < 10; ++i)
            cout << (i+1) << ". " << idx.doc_name(ranking[i].first)
                 << " " << ranking[i].second << endl;

        cout << endl;
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
     
    std::cout << "Finished at " << std::ctime(&end_time)
              << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    return 0;
}
