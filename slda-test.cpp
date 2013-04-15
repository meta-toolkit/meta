/**
 * @file slda-test.cpp
 */

#include <vector>
#include <string>
#include <iostream>
#include "index/document.h"
#include "io/config_reader.h"
#include "topics/slda.h"

using std::vector;
using std::unordered_map;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace meta;
using namespace meta::index;
using namespace meta::topics;
using namespace meta::tokenizers;

void tokenize(vector<Document> & docs, const unordered_map<string, string> & config)
{
    std::shared_ptr<tokenizer> tok = io::config_reader::create_tokenizer(config);

    size_t i = 0;
    for(auto & d: docs)
    {
        common::show_progress(i++, docs.size(), 20, "  tokenizing ");
        tok->tokenize(d, nullptr);
    }
    common::end_progress("  tokenizing ");
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.ini" << endl;
        return 1;
    }

    unordered_map<string, string> config = io::config_reader::read(argv[1]);
    string prefix = config["prefix"] + config["dataset"];
    vector<Document> docs = Document::loadDocs(prefix + "/full-corpus.txt", prefix);

    tokenize(docs, config);
    slda model(config["slda"], 0.1);
    model.estimate(docs);

    return 0;
}
