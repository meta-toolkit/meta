/**
 * @file search.cpp
 */

#include <vector>
#include <string>
#include <iostream>

#include "io/config_reader.h"
#include "tokenizers/tokenizer.h"
#include "index/document.h"
#include "index/inverted_index.h"

using namespace meta;
using std::cerr;
using std::endl;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    cerr << "[Main] Reading config file" << endl;

    auto config = io::config_reader::read(argv[1]);
    std::string prefix = *cpptoml::get_as<std::string>(config, "prefix")
        + *cpptoml::get_as<std::string>(config, "dataset");
    std::string corpus_file = prefix 
        + "/" 
        + *cpptoml::get_as<std::string>(config, "list")
        + "-full-corpus.txt";

    cerr << "[Main] Loading docs" << endl;
    std::vector<index::document> docs = index::document::load_docs(corpus_file, prefix);

    cerr << "[Main] Creating tokenizer(s)" << endl;
    std::shared_ptr<tokenizers::tokenizer> tok = io::config_reader::create_tokenizer(config);

    index::inverted_index idx{"my-index", docs, tok};
    cerr << idx.term_freq(0, 0) << endl;
    cerr << idx.term_freq(5, 3) << endl;

    return 0;
}
