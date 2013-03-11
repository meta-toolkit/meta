#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "util/invertible_map.h"

#include "lib/slda/slda.h"
#include "lib/slda/corpus.h"
#include "lib/slda/utils.h"

using std::vector;
using std::unordered_map;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ofstream;

void run_slda(const Tokenizer* tok, const InvertibleMap<string, int> & mapping)
{
    corpus c;
    string data_filename = "lib/slda/test-new/ceeaus-test.dat";
    c.read_data(data_filename);
    settings setting("lib/slda/settings.txt");

    string directory = "slda-output-inf";
    make_directory(directory);

    string model_filename = "lib/slda/ceeaus-est/final.model";
    slda model;
    model.load_model(model_filename);
    model.infer_only(&c, &setting, directory);

    vector<vector<pair<int, double>>> dists = model.top_terms();
    size_t d = 1;
    for(auto & dist: dists)
    {
        cout << "Top terms for " << mapping.getKeyByValue(d++) << endl;
        for(size_t i = 0; i < 25; ++i)
            cout << "  " << tok->getLabel(dist[i].first) << " " << dist[i].second << endl;
    }
}

int main(int argc, char* argv[])
{

    InvertibleMap<string, int> mapping;
    unordered_map<string, string> config = ConfigReader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];
    vector<Document> documents = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    Tokenizer* tokenizer = ConfigReader::create_tokenizer(config);
    ofstream out("slda-input.dat");

    for(size_t i = 0; i < documents.size(); ++i)
    {
        tokenizer->tokenize(documents[i], nullptr);
        out << documents[i].getLearningData(mapping, true /* using sLDA */);
        if(i % 20 == 0)
            cerr << "  tokenizing " << static_cast<double>(i) / documents.size() * 100 << "%     \r"; 
    }

    out.close();

    run_slda(tokenizer, mapping);

    delete tokenizer;
}
