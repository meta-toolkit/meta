/**
 * @file search.cpp
 */

#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "io/config_reader.h"
#include "tokenizers/tokenizer.h"
#include "index/document.h"
#include "cluster/similarity.h"

using std::pair;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::unordered_map;

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    auto config = io::config_reader::read(argv[1]);
    string prefix = *cpptoml::get_as<std::string>( config, "prefix" )
        + *cpptoml::get_as<std::string>( config, "data" );
    string corpus_file = prefix 
        + "/" 
        + *cpptoml::get_as<std::string>( config, "list" )
        + "-full-corpus.txt";

    std::shared_ptr<tokenizers::tokenizer> tok = io::config_reader::create_tokenizer(config);

    vector<index::document> docs = index::document::load_docs(corpus_file, prefix);

    cerr << "Tokenizing..." << endl;
    for(auto & doc: docs)
        tok->tokenize(doc);

    cerr << "Computing similarities..." << endl;
    vector<pair<string, double>> scores;
    scores.reserve(docs.size() * docs.size());
    for(size_t i = 0; i < docs.size(); ++i)
    {
        cerr << "  " << docs.size() - i - 1 << " remaining    \r";
        for(size_t j = 0; j < i; ++j)
        {
            string comp = docs[i].name() + " " + docs[j].name();
            // heuristically guess if the assignment is completed
            if(docs[i].length() > 10 && docs[j].length() > 10)
                scores.push_back(make_pair(comp, index::document::cosine_similarity(docs[i], docs[j])));
        }
    }
    cerr << endl;

    std::sort(scores.begin(), scores.end(), [](const pair<string, double> & a, const pair<string, double> & b){
        return a.second > b.second;
    });

    for(size_t i = 0; i < scores.size(); ++i)
        cout << scores[i].second << " " << scores[i].first << endl;

    return 0;
}
