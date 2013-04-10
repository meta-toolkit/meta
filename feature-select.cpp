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
#include "classify/select_chi_square.h"
#include "classify/select_info_gain.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ofstream;

using namespace meta;
using namespace meta::index;
using namespace meta::util;
using namespace meta::tokenizers;

void run_liblinear(const string & datafile, const string & prefix)
{
    string command = prefix + "lib/liblinear-1.92/train -s 2 -v 2 -q " + datafile;
    system(command.c_str());
}

void run_selected_features(const vector<Document> & documents,
        const unordered_set<TermID> & features, InvertibleMap<string, int> & mapping, const string & prefix)
{
    string datafile = "liblinear-selected.dat";
    ofstream out(datafile);
    for(auto & d: documents)
        out << d.getFilteredLearningData(mapping, features);

    out.close();
    run_liblinear(datafile, prefix);
}

InvertibleMap<string, int> tokenize(std::shared_ptr<tokenizer> & tokenizer, vector<Document> & documents)
{
    InvertibleMap<string, int> mapping;
    ofstream liblinear_out("liblinear-input.dat");
    size_t i = 0;
    for(auto & d: documents)
    {
        common::show_progress(i++, documents.size(), 20, "  tokenizing ");
        tokenizer->tokenize(d, nullptr);
        liblinear_out << d.getLearningData(mapping, false /* using liblinear */);
    }
    common::end_progress("  tokenizing ");
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

    unordered_map<string, string> config = io::config_reader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];

    vector<Document> documents = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    std::shared_ptr<tokenizer> tok = io::config_reader::create_tokenizer(config);
    InvertibleMap<string, int> mapping = tokenize(tok, documents);

    classify::select_info_gain ig;
    classify::select_chi_square cs;
    classify::select_doc_freq df;
    classify::select_slda slda;
    
    cerr << " Info Gain" << endl;
    vector<pair<TermID, double>> info_features = ig.select(documents);
    cerr << " Chi Square" << endl;
    vector<pair<TermID, double>> chi_features  = cs.select(documents);
    cerr << " Doc Freq" << endl;
    vector<pair<TermID, double>> freq_features = df.select(documents);
    cerr << " sLDA" << endl;
    vector<pair<TermID, double>> slda_features = slda.select(documents);

    vector<vector<pair<TermID, double>>> all_features = {info_features, chi_features, freq_features, slda_features};

    run_liblinear("liblinear-input.dat", path);
    for(double d = 0.01; d < 1.0; d += .02)
    {
        size_t num_features = d * all_features[0].size();
        cout << "Using " << num_features << " features (" << (d * 100) << "%)" << endl;

        for(auto & fs: all_features)
        {
            unordered_set<TermID> features;
            for(size_t i = 0; i < num_features; ++i)
                features.insert(fs[i].first);
            run_selected_features(documents, features, mapping, path);
        }
    }

    return 0;
}
