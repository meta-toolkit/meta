#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "util/invertible_map.h"

#include "classify/select_slda.h"
#include "classify/select_doc_freq.h"

#include "lib/slda/slda.h"
#include "lib/slda/corpus.h"
#include "lib/slda/utils.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ofstream;

void run_liblinear(const string & datafile, const string & prefix)
{
    string command = prefix + "lib/liblinear-1.92/train -s 2 -v 2 -q " + datafile;
    system(command.c_str());
}

void run_slda_plus_svm(const vector<Document> & documents,
        const unordered_set<TermID> & features, InvertibleMap<string, int> & mapping, const string & prefix)
{
    string datafile = "liblinear-selected.dat";
    ofstream out(datafile);
    for(auto & d: documents)
        out << d.getFilteredLearningData(mapping, features);

    out.close();
    run_liblinear(datafile, prefix);
}

vector<TermID> run_slda(const string & prefix)
{
    corpus train_corpus;
    train_corpus.read_data("slda-train.dat");
    settings setting(prefix + "lib/slda/settings.txt");

    string directory = "slda-output-est";
    make_directory(directory);

    // run sLDA
    slda model;
    model.init(setting.alpha, setting.num_topics, &train_corpus);
    model.v_em(&train_corpus, &setting, setting.init_method, directory);

    // evaluate the sLDA model
    corpus test_corpus;
    test_corpus.read_data("slda-test.dat");
    string model_filename = "slda-output-est/final.model";
    model.load_model(model_filename);
    model.infer_only(&test_corpus, &setting, directory);

    vector<pair<TermID, double>> features;
    vector<vector<pair<int, double>>> dists = model.top_terms();

    cerr << dists.front().size() << " total features" << endl;

    for(auto & dist: dists)
        for(auto & p: dist)
            features.push_back(std::make_pair(static_cast<TermID>(p.first), p.second));

    std::sort(features.begin(), features.end(), [](const pair<TermID, double> & a, const pair<TermID, double> b) {
        return a.second > b.second;
    });

    vector<TermID> selected_features;
    unordered_set<TermID> seen;
    selected_features.reserve(features.size());
    for(auto & p: features)
    {
        if(seen.find(p.first) == seen.end())
        {
            selected_features.push_back(p.first);
            seen.insert(p.first);
        }
    }

    return selected_features;
}

InvertibleMap<string, int> tokenize(std::shared_ptr<Tokenizer> & tokenizer, vector<Document> & documents)
{
    InvertibleMap<string, int> mapping;

    ofstream slda_train_out("slda-train.dat");
    ofstream slda_test_out("slda-test.dat");
    ofstream liblinear_out("liblinear-input.dat");

    for(size_t i = 0; i < documents.size(); ++i)
    {
        tokenizer->tokenize(documents[i], nullptr);
        liblinear_out << documents[i].getLearningData(mapping, false /* using liblinear */);

        // have sLDA train on half, test on half
        if(i % 2 == 0)
            slda_train_out << documents[i].getLearningData(mapping, true /* using sLDA */);
        else
            slda_test_out << documents[i].getLearningData(mapping, true /* using sLDA */);
        
        if(i % 20 == 0)
            cerr << "  tokenizing " << static_cast<double>(i) / documents.size() * 100 << "%     \r"; 
    }
    cerr << "  tokenizing 100%\r       ";

    slda_train_out.close();
    slda_test_out.close();
    liblinear_out.close();

    return mapping;
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cerr << "Usage:\t" << argv[0] << " config.ini prefix" << endl;
        return 1;
    }

    string path(argv[2]);

    unordered_map<string, string> config = ConfigReader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];

    vector<Document> documents = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    std::shared_ptr<Tokenizer> tokenizer = ConfigReader::create_tokenizer(config);
    InvertibleMap<string, int> mapping = tokenize(tokenizer, documents);

    vector<TermID> selected_features = run_slda(path);
    run_liblinear("liblinear-input.dat", path);
    for(double d = 0.01; d < 0.5; d += .02)
    {
        size_t num_features = d * selected_features.size();
        cout << "Using " << num_features << " features (" << (d * 100) << "%)" << endl;
        unordered_set<TermID> features(selected_features.begin(), selected_features.begin() + num_features);
        run_slda_plus_svm(documents, features, mapping, path);
    }

    return 0;
}
