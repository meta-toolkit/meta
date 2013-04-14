/**
 * @file select_slda.cpp
 */

#include <unordered_map>
#include <utility>
#include "../lib/slda/slda.h"
#include "../lib/slda/corpus.h"
#include "../lib/slda/utils.h"
#include "classify/select_slda.h"

namespace meta {
namespace classify {

using std::unordered_map;
using std::pair;
using std::vector;
using index::Document;

select_slda::select_slda(const vector<Document> & docs):
    _docs(docs) { /* nothing */ }

vector<pair<term_id, double>> select_slda::select()
{
    // create sLDA input data
    util::InvertibleMap<class_label, int> mapping;
    std::ofstream slda_train_out("slda-train.dat");
    for(auto & d: _docs)
        slda_train_out << d.getLearningData(mapping, true /* using sLDA */);
    slda_train_out.close();

    // initialize sLDA framework
    corpus train_corpus;
    train_corpus.read_data("slda-train.dat");
    std::string directory = "slda-output-est";
    make_directory(directory);
    settings setting("slda-settings.txt");

    // run sLDA
    class slda model;
    model.init(setting.alpha, setting.num_topics, &train_corpus);
    model.v_em(&train_corpus, &setting, setting.init_method, directory);

    // collect features
    vector<vector<pair<int, double>>> dists = model.top_terms();
    unordered_map<term_id, double> feature_weights;
    for(auto & dist: dists)
    {
        for(auto & p: dist)
        {
            term_id id = static_cast<term_id>(p.first);
            // 0 will actually be higher than any feature rating from sLDA
            if(feature_weights[id] == 0.0 || feature_weights[id] < p.second)
                feature_weights[id] = p.second;
        }
    }

    // sort features
    vector<pair<term_id, double>> features(feature_weights.begin(), feature_weights.end());
    std::sort(features.begin(), features.end(),
        [](const pair<term_id, double> & a, const pair<term_id, double> & b) {
            return a.second > b.second;
        }
    );

    return features;
}

std::unordered_map<class_label, std::vector<std::pair<term_id, double>>>
select_slda::select_by_class() {
    return {};
}


}
}
