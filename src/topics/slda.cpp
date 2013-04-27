/**
 * @file slda.cpp
 * @author Sean Massung
 */

#include <fstream>
#include <unordered_set>
#include "topics/slda.h"

namespace meta {
namespace topics {

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::string;
using util::invertible_map;
using index::document;

slda::slda(const string & slda_path, double alpha):
    _alpha(alpha),
    _slda_path(slda_path),
    _mapping(invertible_map<class_label, int>())
{ /* nothing */ }

void slda::estimate(const vector<document> & docs)
{
    size_t num_classes = create_input_files(docs);

    string command = _slda_path + "/slda est slda-data slda-labels ";
    command += _slda_path + "/settings.txt "; // use default settings for now
    command += common::to_string(_alpha) + " ";
    command += common::to_string(num_classes) + " ";
    command += "random slda-est-output";
    command += " 2>&1> /dev/null";
    system(command.c_str());
}

unordered_map<class_label, vector<pair<term_id, double>>> slda::class_distributions() const
{
    unordered_map<class_label, vector<pair<term_id, double>>> dists;

    vector<vector<double>> probs = get_probs();
    int curr_label_id = 1; // start at one since mappings start at one
    for(auto & dist: probs)
    {
        vector<pair<term_id, double>> this_dist;
        term_id curr_id = 0;

        // combine each probability with its term id
        for(auto & prob: dist)
        {
            this_dist.push_back(std::make_pair(curr_id, prob));
            ++curr_id;
        }

        // sort this distribution by term weight
        std::sort(this_dist.begin(), this_dist.end(),
            [](const pair<term_id, double> & a, const pair<term_id, double> & b) {
                return a.second > b.second;
            }
        );

        // assign the current distribution to its respective class
        dists[_mapping.get_key(curr_label_id)] = this_dist;
        ++curr_label_id;
    }

    return dists;
}

vector<vector<double>> slda::get_probs() const
{
    // read the betas from the file
    std::ifstream in("slda-est-output/final.model.text");
    string line;
    vector<string> str_probs;
    while(in.good())
    {
        // skip to the betas (term probabilities per class)
        std::getline(in, line);
        if(line == "betas: ")
        {
            // grab all the betas
            std::getline(in, line);
            while(line != "etas: ")
            {
                str_probs.push_back(line);
                std::getline(in, line);
            }

            // once we've gotten the betas, we're done
            break;
        }
    }
    in.close();

    // convert them into doubles for each class (order in outer vector is the
    // class id)
    vector<vector<double>> probs;
    for(auto & str: str_probs)
    {
        double prob;
        vector<double> curr_probs;
        std::istringstream iss(str);
        while(iss >> prob)
            curr_probs.push_back(prob);
        probs.emplace_back(curr_probs);
    }

    return probs;
}

vector<pair<term_id, double>> slda::select() const
{
    unordered_map<term_id, double> weights;

    // find highest weighted terms
    vector<vector<double>> probs = get_probs();
    for(auto & dist: probs)
    {
        term_id curr_id = 0;
        for(auto & prob: dist)
        {
            // 0 will actually be higher than any feature rating from sLDA
            if(weights[curr_id] == 0.0 || weights[curr_id] < prob)
                weights[curr_id] = prob;
            ++curr_id;
        }
    }

    // sort by term weight
    vector<pair<term_id, double>> ret(weights.begin(), weights.end());
    std::sort(ret.begin(), ret.end(),
        [](const pair<term_id, double> & a, const pair<term_id, double> & b) {
            return a.second > b.second;
        }
    );

    return ret;
}

size_t slda::create_input_files(const vector<document> & docs)
{
    std::ofstream data("slda-data");
    std::ofstream labels("slda-labels");
    unordered_set<class_label> unique_labels;

    for(auto & d: docs)
    {
        labels << d.get_slda_label_data(_mapping);
        data << d.get_slda_term_data();
        unique_labels.insert(d.label());
    }

    labels.close();
    data.close();

    return unique_labels.size();
}

void slda::infer(const vector<document> & docs)
{
    create_input_files(docs);

    string command = _slda_path + "/slda inf slda-data slda-labels ";
    command += _slda_path + "/settings.txt "; // use default settings for now
    command += "slda-est-output/final.model slda-inf-output ";
    command += "2>&1> /dev/null";
    system(command.c_str());
}

}
}
