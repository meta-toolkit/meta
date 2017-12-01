#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "meta/learn/loss/all.h"
#include "meta/learn/loss/hinge.h"
#include "meta/learn/loss/loss_function.h"
#include "meta/learn/loss/loss_function_factory.h"
#include "meta/learn/sgd.h"
#include "meta/learn/instance.h"

using namespace meta;
using namespace std;

enum DATA_TYPE {
    TRAINING,
    VALIDATION,
    TESTING
};

int main(int argc, char* argv[])
{

    std::cerr << "Hello LETOR!" << std::endl;

    if (argc != 2) {
        std::cerr << "Please specify exactly one directory path for training, validation and testing sets" << std::endl;
    }

    std::unique_ptr<learn::loss::loss_function> loss = learn::loss::make_loss_function(learn::loss::hinge::id.to_string());
    learn::sgd_model *model = new learn::sgd_model(46);

    //training phase
    vector<vector<vector<feature_vector*>*>*> *training_dataset = new vector<vector<vector<feature_vector*>*>*>();
    read_data(TRAINING, argv[1], training_dataset);

    //validation phase
    vector<vector<vector<feature_vector*>*>*> *validation_dataset = new vector<vector<vector<feature_vector*>*>*>();
    vector<vector<vector<string>*>*>*> *validation_docids = new vector<vector<vector<string>*>*>*>();
    read_data(VALIDATION, argv[1], validation_dataset, validation_docids);

    //testing phase
    vector<vector<vector<feature_vector*>*>*> *testing_dataset = new vector<vector<vector<feature_vector*>*>*>();
    vector<vector<vector<string>*>*>*> *testing_docids = new vector<vector<vector<string>*>*>*>();
    read_data(TESTING, argv[1], testing_dataset, testing_docids);

    loss.reset();
    delete model;
//    free_dataset(training_dataset);
//    free_dataset(validation_dataset);
//    free_docids(validation_docids);
//    free_dataset(testing_dataset);
//    free_docids(testing_docids);
    std::cerr << "Bye LETOR!" << std::endl;

    return 0;
}

void read_data(DATA_TYPE data_type, string data_dir,
               vector<vector<vector<feature_vector*>*>*> *dataset,
                vector<vector<vector<string>*>*>*> *docids) {
    string data_file = data_dir;
    switch(data_type) {
        case TRAINING:
            data_file += "/train.txt";
            break;
        case VALIDATION:
            data_file += "/vali.txt";
            break;
        case TESTING:
            data_file += "/test.txt";
            break;
    }
    std::ifstream infile(data_file);
    string line;
    int feature_idx;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        int label, qid, feature_id;
        double feature_val;
        string tmp_str, docid;
        iss >> label >> tmp_str;
        stringstream ss(tmp_str.substr(tmp_str.find(':') + 1));
        ss >> qid;

        vector<vector<feature_vector*>*> *label_dataset;
        if (dataset->find(qid) != dataset->end()) {
            label_dataset = dataset[qid];
        } else {
            label_dataset = new unordered_map<int, feature_vector*>();
            dataset[qid] = label_dataset;
        }
        feature_vector *features = new feature_vector();
        for (feature_idx = 0; feature_idx < 46; feature_idx++) {
            iss >> tmp_str;
            stringstream ssid(tmp_str.substr(0, tmp_str.find(':')));
            ssid >> feature_id;
            stringstream ssval(tmp_str.substr(tmp_str.find(':') + 1));
            ssval >> feature_val;
            (*features)[ssid] = ssval;
        }
        iss >> tmp_str;
        iss >> tmp_str;
        iss >> docid;
    }
}