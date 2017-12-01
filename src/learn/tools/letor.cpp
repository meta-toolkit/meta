#include <functional>
#include <iostream>
#include <vector>
#include <random>
#include <unordered_map>

#include <fstream>
#include <sstream>

#include "meta/learn/loss/all.h"
#include "meta/learn/loss/hinge.h"
#include "meta/learn/loss/loss_function.h"
#include "meta/learn/loss/loss_function_factory.h"
#include "meta/learn/sgd.h"
#include "meta/learn/instance.h"


using namespace meta;
using namespace learn;
using namespace std;
using namespace util;

using tupl = std::tuple<feature_vector, int, int>;
//using feature_id = term_id;
//using feature_vector = util::sparse_vector<feature_id, double>;

enum DATA_TYPE {
    TRAINING,
    VALIDATION,
    TESTING
};

void read_data(DATA_TYPE data_type, string data_dir, vector<int> *qids,
               unordered_map<int, unordered_map<int, vector<feature_vector>*>*> *dataset);
int train(string data_dir);

int main(int argc, char* argv[])
{

    std::cerr << "Hello LETOR!" << std::endl;

    if (argc != 2) {
        std::cerr << "Please specify exactly one directory path for training, validation and testing sets" << std::endl;
    }

    //training phase
    train(argv[1]);

//    //validation phase
//    vector<int> *validation_qids = new vector<int>();
//    unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *validation_dataset = new unordered_map<int, unordered_map<int, vector<feature_vector*>*>*>();
//    unordered_map<int, unordered_map<int, vector<string>*>*> *validation_docids = new unordered_map<int, unordered_map<int, vector<string>*>*>();
//    read_data(VALIDATION, argv[1], validation_qids, validation_dataset, validation_docids);
//
//    //testing phase
//    vector<int> *testing_qids = new vector<int>();
//    unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *testing_dataset = new unordered_map<int, unordered_map<int, vector<feature_vector*>*>*>();
//    unordered_map<int, unordered_map<int, vector<string>*>*> *testing_docids = new unordered_map<int, unordered_map<int, vector<string>*>*>();
//    read_data(TESTING, argv[1], testing_qids, testing_dataset, testing_docids);

//    free_dataset(training_dataset);
//    free_dataset(validation_dataset);
//    free_docids(validation_docids);
//    free_dataset(testing_dataset);
//    free_docids(testing_docids);
    std::cerr << "Bye LETOR!" << std::endl;

    return 0;
}

/**
 * Return a random pair of tuple for training the svm classifier
 * @param indexed_dataset
 * @return
 */
std::pair<tupl, tupl> getRandomPair(vector<int> *training_qids,
                                    unordered_map<int, unordered_map<int, vector<feature_vector>*>*> *training_dataset) {
    std::default_random_engine generator;

    //select q uniformly at random from Q
    int max_q = training_qids->size();
    std::uniform_int_distribution<int> qid_distribution(0, max_q);
    int q_index = qid_distribution(generator);

    int qid = (*training_qids)[q_index];
    unordered_map<int, vector<feature_vector>*> *qid_vec = (*training_dataset)[qid];



    //select ya uniformly at random from Y [q]
    int max_ya = qid_vec->size();
    std::uniform_int_distribution<int> ya_distribution(0, max_ya);
    int ya_index = ya_distribution(generator);

    //select (a, ya, q) uniformly at random from P[q][ya]
    int max_a = (*qid_vec)[ya_index]->size();
    std::uniform_int_distribution<int> a_distribution(0, max_a);
    int a_index = a_distribution(generator);
    feature_vector a = (*(*qid_vec)[ya_index])[a_index];

    tupl d1  = std::make_tuple(a, ya_index, qid);


    qid_vec->erase(ya_index);
    //select yb uniformly at random from Y [q] − ya.
    int max_yb = max_ya -1;
    std::uniform_int_distribution<int> yb_distribution(0, max_yb);
    int yb_index = yb_distribution(generator);

    //select (b, yb, q) uniformly at random from P[q][yb]
    int max_b = (*qid_vec)[yb_index]->size();
    std::uniform_int_distribution<int> b_distribution(0, max_b);
    int b_index = b_distribution(generator);
    feature_vector b = (*(*qid_vec)[yb_index])[b_index];

    tupl d2  = std::make_tuple(b, yb_index, qid);

    return std::make_pair(d1,d2);
}


/**
 * Train the pairwise ranker model
 * @return
 */
int train(string data_dir) {
    vector<int> *training_qids = new vector<int>();
    unordered_map<int, unordered_map<int, vector<feature_vector>*>*> *training_dataset
            = new unordered_map<int, unordered_map<int, vector<feature_vector>*>*>();
    read_data(TRAINING, data_dir, training_qids, training_dataset);

    int n_iter = 1000000;

    learn::sgd_model *model = new learn::sgd_model(46);

    std::unique_ptr<learn::loss::loss_function> loss
            = learn::loss::make_loss_function(learn::loss::hinge::id.to_string());

    for (int i = 0; i < n_iter; i++) {
        std::pair<tupl, tupl> data_pair = getRandomPair(training_qids, training_dataset);

        feature_vector a, b;
        int y_a, y_b, qid;

        std::tie(a, y_a, qid) = data_pair.first;

        std::tie(b, y_b, qid) = data_pair.second;

        feature_vector &x = a;
        x -= b;

        double expected_label = y_a - y_b;

        model->train_one(x, expected_label, *loss);

    }

    return 0;
}




void read_data(DATA_TYPE data_type, string data_dir, vector<int> *qids,
               unordered_map<int, unordered_map<int, vector<feature_vector>*>*> *dataset) {
    string data_file = data_dir;
    switch (data_type) {
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

        unordered_map<int, vector<feature_vector> *> *query_dataset;
        if (dataset->find(qid) != dataset->end()) {
            query_dataset = (*dataset)[qid];
        } else {
            qids->push_back(qid);
            query_dataset = new unordered_map<int, vector<feature_vector>*>();
            (*dataset)[qid] = query_dataset;
        }
        vector<feature_vector> *label_dataset;
        if (query_dataset->find(label) != query_dataset->end()) {
            label_dataset = (*query_dataset)[label];
        } else {
            label_dataset = new vector<feature_vector>();
            (*query_dataset)[label] = label_dataset;
        }
        learn::feature_vector features;
        for (feature_idx = 0; feature_idx < 46; feature_idx++) {
            iss >> tmp_str;
            stringstream ssid(tmp_str.substr(0, tmp_str.find(':')));
            ssid >> feature_id;
            stringstream ssval(tmp_str.substr(tmp_str.find(':') + 1));
            ssval >> feature_val;
            features[term_id{feature_id}] = feature_val;
        }
        label_dataset->push_back(features);
        iss >> tmp_str;
        iss >> tmp_str;
        iss >> docid;
    }
}