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

InvertibleMap<string, int> tokenize(std::shared_ptr<Tokenizer> & tokenizer, vector<Document> & documents)
{
    InvertibleMap<string, int> mapping;
    ofstream liblinear_out("liblinear-input.dat");
    size_t i = 0;
    for(auto & d: documents)
    {
        Common::show_progress(i++, documents.size(), 20, "  tokenizing ");
        tokenizer->tokenize(d, nullptr);
        liblinear_out << d.getLearningData(mapping, false /* using liblinear */);
    }
    Common::end_progress("  tokenizing ");
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

    vector<pair<TermID, double>> selected_features = classify::feature_select::slda(documents);
    run_liblinear("liblinear-input.dat", path);
    for(double d = 0.01; d < 0.5; d += .02)
    {
        size_t num_features = d * selected_features.size();
        cout << "Using " << num_features << " features (" << (d * 100) << "%)" << endl;
        unordered_set<TermID> features;
        for(size_t i = 0; i < num_features; ++i)
            features.insert(selected_features[i].first);
        run_slda_plus_svm(documents, features, mapping, path);
    }

    return 0;
}
