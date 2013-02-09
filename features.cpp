/**
 * @file features.cpp
 */

#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <map>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "io/parser.h"
#include "util/common.h"

using std::unordered_map;
using std::shared_ptr;
using std::pair;
using std::vector;
using std::cout;
using std::endl;
using std::string;

string getClass(const string & path)
{
    size_t idx = path.find_first_of("/");
    return path.substr(0, idx);
}

unordered_map<string, vector<Document>> getDocs(const string & path)
{
    unordered_map<string, vector<Document>> docs;
    Parser parser(path + "/full-corpus.txt", "\n");
    while(parser.hasNext())
    {
        string file = parser.next();
        string className = getClass(file);
        docs[className].push_back(Document(path + "/" + file));
    }
    return docs;
}

void combine_counts(unordered_map<TermID, unsigned int> & language_model, const
        unordered_map<TermID, unsigned int> & doc_counts)
{
    for(auto & count: doc_counts)
        language_model[count.first] += count.second;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    unordered_map<string, string> config = ConfigReader::read(argv[1]);
    unordered_map<string, vector<Document>> docs =
        getDocs("/home/sean/projects/senior-thesis-data/" + config["prefix"]);

    int nVal;
    istringstream(config["ngram"]) >> nVal;

    unordered_map<string, NgramTokenizer::NgramType> ngramOpt = {
        {"POS", NgramTokenizer::POS}, {"Word", NgramTokenizer::Word},
        {"FW", NgramTokenizer::FW}, {"Char", NgramTokenizer::Char}
    };

    unordered_map<string, TreeTokenizer::TreeTokenizerType> treeOpt = {
        {"Subtree", TreeTokenizer::Subtree}, {"Depth", TreeTokenizer::Depth},
        {"Branch", TreeTokenizer::Branch}, {"Tag", TreeTokenizer::Tag}
    };
  
    Tokenizer* class_tokenizer = NULL; 
    string method = config["method"];
    cerr << "ngramOpt: " << config["ngramOpt"] << endl;
    if(method == "ngram")
        class_tokenizer = new NgramTokenizer(nVal, ngramOpt[config["ngramOpt"]]);
    else if(method == "tree")
        class_tokenizer = new TreeTokenizer(treeOpt[config["treeOpt"]]);
    else
    {
        cerr << "Method was not able to be determined" << endl;
        return 1;
    } 

    cerr << "Tokenizing..." << endl;
    shared_ptr<unordered_map<TermID, unsigned int>> bg_model(new unordered_map<TermID, unsigned int>);
    unordered_map<string, unordered_map<TermID, unsigned int>> language_models;
    for(auto & str: docs)
    {
        for(auto & doc: str.second)
        {
            class_tokenizer->tokenize(doc, bg_model);
            combine_counts(language_models[str.first], doc.getFrequencies());
        }
    }

    cerr << "Smoothing..." << endl;
    unordered_map<string, unordered_map<TermID, double>> smoothed_models;
    for(auto & model: language_models)
    {
        size_t total = 0;
        for(auto & term: model.second)
        {
            total += term.second;
            smoothed_models[model.first][term.first] = (static_cast<double>(term.second) + 1.0) /
                ((*bg_model)[term.first] + 2 * bg_model->size());
        }

        map<double, TermID> sorted;
        for(auto & m: smoothed_models[model.first])
            sorted.insert(make_pair(m.second, m.first));

        cout << model.first << endl;
        //size_t num = 16;
        map<double, TermID>::reverse_iterator p = sorted.rbegin();
        for( ; p != sorted.rend() /*&& num != 0*/; ++p/*, --num*/)
        {
            cout << " " << p->first << " " << class_tokenizer->getLabel(p->second)
                 << " p(w|class) = "
                 << static_cast<double>(language_models[model.first][p->second]) / total << endl;
        }
    }

    delete class_tokenizer;
    return 0;
}
