/**
 * @file learn.cpp
 * This creates input for liblinear based on features extracted from my
 *  tokenizers.
 */

#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "util/common.h"
#include "util/invertible_map.h"

using std::vector;
using std::string;
using std::unordered_map;

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    unordered_map<string, string> config = io::config_reader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];
    util::InvertibleMap<string, int> mapping; // for unique ids when printing liblinear data

    vector<index::Document> documents = index::Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    std::shared_ptr<tokenizers::tokenizer> tok = io::config_reader::create_tokenizer(config);

    for(size_t i = 0; i < documents.size(); ++i)
    {
        common::show_progress(i, documents.size(), 20, "  tokenizing ");
        // order of lines in the liblinear input file does NOT matter (tested)
        tok->tokenize(documents[i], nullptr);
        std::cout << documents[i].getLearningData(mapping, false /* using liblinear */);
    }
    common::end_progress("  tokenizing ");

    return 0;
}
