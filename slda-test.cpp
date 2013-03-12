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

void run_liblinear(const string & datafile, const string & name)
{
    cerr << endl;
    cerr << string(16, '*') << name << string(16, '*') << endl;
    cerr << endl;
    string command = "lib/liblinear-1.92/train -s 2 -v 2 -q " + datafile;
    system(command.c_str());
}

void run_slda_plus_svm(const vector<Document> & documents,
        const unordered_set<TermID> & features, InvertibleMap<string, int> & mapping)
{
    string datafile = "liblinear-selected.dat";
    ofstream out(datafile);
    for(auto & d: documents)
        out << d.getFilteredLearningData(mapping, features);

    out.close();
    run_liblinear(datafile, " sLDA+SVM ");
    cerr << endl;
}

unordered_set<TermID> run_slda(const std::shared_ptr<Tokenizer> tok, const InvertibleMap<string, int> & mapping, size_t num_features)
{
    cerr << endl;
    cerr << string(16, '*') << " sLDA " << string(16, '*') << endl;
    cerr << endl;

    corpus train_corpus;
    train_corpus.read_data("slda-train.dat");
    settings setting("lib/slda/settings.txt");

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

    unordered_set<TermID> selected_features;
    vector<vector<pair<int, double>>> dists = model.top_terms();
    //size_t d = 1;
    for(auto & dist: dists)
    {
        //cout << "Top terms for " << mapping.getKeyByValue(d++) << endl;
        for(size_t i = 0; i < num_features; ++i)
        {
            //cout << "  " << tok->getLabel(dist[i].first) << " " << dist[i].second << endl;
            selected_features.insert(static_cast<TermID>(dist[i].first));
        }
    }

    return selected_features;
}

int main(int argc, char* argv[])
{
    InvertibleMap<string, int> mapping;
    unordered_map<string, string> config = ConfigReader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];
    vector<Document> documents = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    std::shared_ptr<Tokenizer> tokenizer = ConfigReader::create_tokenizer(config);

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
    cerr << "  tokenizing 100%       " << endl;

    slda_train_out.close();
    slda_test_out.close();
    liblinear_out.close();

    size_t num_features = atoi(argv[2]);
    unordered_set<TermID> selected_features = run_slda(tokenizer, mapping, num_features);
    run_liblinear("liblinear-input.dat", " SVM ");
    run_slda_plus_svm(documents, selected_features, mapping);
}
