/**
 * @file interactive_search.cpp
 */

#include <vector>
#include <string>
#include <iostream>
#include "util/common.h"
#include "tokenizers/tokenizer.h"
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/ranker/all.h"
#include "caching/all.h"

using namespace meta;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    auto idx = index::make_index<index::inverted_index,
                                 caching::splay_cache>(argv[1], uint32_t{10000});

    index::pivoted_length ranker;
    //index::okapi_bm25 ranker;
    
    cout << "Enter a query, or type \"exit\" to quit." << endl << endl;

    std::string text;
    while(true)
    {
        cout << "> ";
        std::getline(cin, text);

        if(text == "exit" || text == "quit")
            break;
            
        corpus::document query{"[user input]", doc_id{0}};
        query.set_content(text);

        std::vector<std::pair<doc_id, double>> ranking;
        auto time = common::time([&](){
            ranking = ranker.score(idx, query);
        });
        
        cout << "Showing top 10 of " << ranking.size()
             << " results (" << time.count() << "ms)" << endl;
        for(size_t i = 0; i < ranking.size() && i < 10; ++i)
            cout << (i+1) << ". " << idx.doc_name(ranking[i].first)
                 << " " << ranking[i].second << endl;

        cout << endl;
    }
}
