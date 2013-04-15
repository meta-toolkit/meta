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
#include "classify/select_corr.h"
#include "classify/select_odds.h"

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
        const unordered_set<term_id> & features, InvertibleMap<string, int> & mapping, const string & prefix)
{
    string datafile = "liblinear-selected.dat";
    ofstream out(datafile);
    for(auto & d: documents)
        out << d.get_filtered_liblinear_data(mapping, features);

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
        liblinear_out << d.get_liblinear_data(mapping);
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
    string prefix = config["prefix"] + config["dataset"];

    vector<Document> documents = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    std::shared_ptr<tokenizer> tok = io::config_reader::create_tokenizer(config);
    InvertibleMap<string, int> mapping = tokenize(tok, documents);

//  cerr << " Info Gain" << endl;
//  classify::select_info_gain ig(documents);
//  cerr << " Chi Square" << endl;
//  classify::select_chi_square cs(documents);
//  cerr << " Doc Freq" << endl;
//  classify::select_doc_freq df(documents);
    cerr << " Correlation Coefficient" << endl;
    classify::select_corr_coeff cc(documents);
//  cerr << " Odds Ratio" << endl;
//  classify::select_odds_ratio od(documents);
//  cerr << " sLDA" << endl;
//  classify::select_slda slda(documents);
   
 // auto info_features = ig.select_by_class();
 // auto chi_features  = cs.select_by_class();
 // auto freq_features = df.select_by_class();
    auto cc_features = cc.select_by_class();
 // auto od_features = od.select_by_class();

 // run_liblinear("liblinear-input.dat", path);
 // for(double d = 0.01; d < 1.0; d += .02)
 // {
 //     size_t num_features = d * all_features[0].size();
 //     cout << "Using " << num_features << " features (" << (d * 100) << "%)" << endl;

        for(auto & fs: cc_features)
        {
            size_t num = 100;
            cout << "-------------------------------------------" << endl;
            cout << "    " << fs.first << endl;
            cout << "-------------------------------------------" << endl;
            for(auto & p: fs.second)
            {
                cout << p.second << " " << tok->label(p.first) << endl;
                if(num-- == 0)
                    break;
            }
 //         unordered_set<term_id> features;
 //         for(size_t i = 0; i < num_features; ++i)
 //             features.insert(fs[i].first);
 //         run_selected_features(documents, features, mapping, path);
        }
 // }

    return 0;
}
