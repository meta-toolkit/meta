/**
 * @file select_slda.cpp
 */

#include <unordered_map>
#include <utility>
#include "../lib/slda/slda.h"
#include "../lib/slda/corpus.h"
#include "../lib/slda/utils.h"
#include "classify/select_slda.h"
#include "classify/select.h"

using std::unordered_map;
using std::string;
using std::pair;
using std::vector;

vector<pair<TermID, double>> classify::feature_select::slda(const vector<Document> & docs)
{
    // create sLDA input data
    InvertibleMap<string, int> mapping;
    std::ofstream slda_train_out("slda-train.dat");
    for(auto & d: docs)
        slda_train_out << d.getLearningData(mapping, true /* using sLDA */);
    slda_train_out.close();

    // initialize sLDA framework
    corpus train_corpus;
    train_corpus.read_data("slda-train.dat");
    string directory = "slda-output-est";
    make_directory(directory);
    settings setting("slda-settings.txt");

    // run sLDA
    class slda model;
    model.init(setting.alpha, setting.num_topics, &train_corpus);
    model.v_em(&train_corpus, &setting, setting.init_method, directory);

    // collect features
    vector<vector<pair<int, double>>> dists = model.top_terms();
    unordered_map<TermID, double> feature_weights;
    for(auto & dist: dists)
    {
        for(auto & p: dist)
        {
            TermID id = static_cast<TermID>(p.first);
            // 0 will actually be higher than any feature rating from sLDA
            if(feature_weights[id] == 0.0 || feature_weights[id] < p.second)
                feature_weights[id] = p.second;
        }
    }

    // sort features
    vector<pair<TermID, double>> features(feature_weights.begin(), feature_weights.end());
    std::sort(features.begin(), features.end(),
        [](const pair<TermID, double> & a, const pair<TermID, double> & b) {
            return a.second > b.second;
        }
    );

    return features;
}
