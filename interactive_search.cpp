/**
 * @file interactive_search.cpp
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "caching/all.h"
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/ranker/all.h"
#include "analyzers/analyzer.h"
#include "util/printing.h"
#include "util/time.h"

using namespace meta;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;

std::string get_snippets(const std::string & filename, const std::string & text)
{
    std::ifstream in{filename};
    std::string str{(std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>()};
    std::string snippets = "";
    std::string word;
    std::istringstream iss{text};

    // for each "word" in query
    while(iss >> word)
    {
        // get all occurences of text
        size_t pos = str.find(word);
        while(pos != std::string::npos)
        {
            std::replace(str.begin(), str.end(), '\n', ' ');
            int begin = std::max(0, static_cast<int>(pos) - 50);
            int end = std::min<int>(static_cast<int>(pos) + 50, str.size());
            snippets += "..." + str.substr(begin, end - begin) + "...\n";
            pos = str.find(word, pos + 1);
        }
    }

    return snippets;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    logging::set_cerr_logging();
    auto idx = index::make_index<index::inverted_index,
                                 caching::splay_cache>(argv[1], uint32_t{10000});

    //index::pivoted_length ranker;
    index::okapi_bm25 ranker;

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
        {
            std::string path{idx.doc_path(ranking[i].first)};
            cout << printing::make_bold(
                        std::to_string(i+1) + ". " + path + " ("
                        + std::to_string(ranking[i].second) + ")"
                    ) << endl;
            cout << get_snippets(path, text) << endl << endl;

        }

        cout << endl;
    }
}
