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
using namespace util;
using namespace std;

using tupl = std::tuple<feature_vector, int, int>;

enum DATA_TYPE {
    TRAINING,
    VALIDATION,
    TESTING
};

bool compare_docscore(const pair<string, double> &p1, const pair<string, double> &p2) {
    return p1.second > p2.second;
}

void read_data(DATA_TYPE data_type, string data_dir, vector<int> *qids,
               unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *dataset,
                unordered_map<int, unordered_map<int, vector<string>*>*> *docids,
                unordered_map<int, unordered_map<string, int>*> *relevance_map);
std::pair<tupl, tupl> getRandomPair(vector<int> *training_qids,
                                    unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *training_dataset,
                                    int random_seed);
void free_dataset(unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *dataset);
int train(string data_dir, sgd_model *model);
void free_docids(unordered_map<int, unordered_map<int, vector<string>*>*> *docids);
void free_relevances(unordered_map<int, unordered_map<string, int>*> *relevances);
void evaluate(vector<int> *qids, unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *dataset,
                                unordered_map<int, unordered_map<int, vector<string>*>*> *docids,
                                unordered_map<int, unordered_map<string, int>*> *relevance_map,
                                sgd_model *model);
int validate(string data_dir, sgd_model *model);
int test(string data_dir, sgd_model *model);

int main(int argc, char* argv[])
{

    std::cerr << "Hello LETOR!" << std::endl;

    if (argc != 2) {
        std::cerr << "Please specify exactly one directory path for training, validation and testing sets" << std::endl;
    }

    learn::sgd_model *model = new learn::sgd_model(46);

    //training phase
    train(argv[1], model);

    //validation phase
    validate(argv[1], model);

    //testing phase
    test(argv[1], model);

    delete model;

    std::cerr << "Bye LETOR!" << std::endl;

    return 0;
}

int test(string data_dir, sgd_model *model) {
    vector<int> *testing_qids = new vector<int>();
    unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *testing_dataset
                                                  = new unordered_map<int, unordered_map<int, vector<feature_vector*>*>*>();
    unordered_map<int, unordered_map<int, vector<string>*>*> *testing_docids
                                                  = new unordered_map<int, unordered_map<int, vector<string>*>*>();
    unordered_map<int, unordered_map<string, int>*> *relevance_map = new unordered_map<int, unordered_map<string, int>*>();
    read_data(TESTING, data_dir, testing_qids, testing_dataset, testing_docids, relevance_map);
    evaluate(testing_qids, testing_dataset, testing_docids, relevance_map, model);

    return 0;
}

int validate(string data_dir, sgd_model *model) {
    vector<int> *validation_qids = new vector<int>();
    unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *validation_dataset
                                                  = new unordered_map<int, unordered_map<int, vector<feature_vector*>*>*>();
    unordered_map<int, unordered_map<int, vector<string>*>*> *validation_docids
                                                  = new unordered_map<int, unordered_map<int, vector<string>*>*>();
    unordered_map<int, unordered_map<string, int>*> *relevance_map = new unordered_map<int, unordered_map<string, int>*>();
    read_data(VALIDATION, data_dir, validation_qids, validation_dataset, validation_docids, relevance_map);
    evaluate(validation_qids, validation_dataset, validation_docids, relevance_map, model);

    return 0;
}

void evaluate(vector<int> *qids, unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *dataset,
                            unordered_map<int, unordered_map<int, vector<string>*>*> *docids,
                            unordered_map<int, unordered_map<string, int>*> *relevance_map,
                            sgd_model *model) {

    for (auto query_iter = dataset->begin(); query_iter != dataset->end(); query_iter++) {
        auto query_dataset = query_iter->second;
        auto query_docids = (*docids)[query_iter->first];
        if (query_dataset->size() <= 1) {
            continue;
        }
        vector<std::pair<string, double>> *doc_scores = new vector<std::pair<string, double>>();
        for (auto label_iter = query_dataset->begin(); label_iter != query_dataset->end(); label_iter++) {
            auto label_dataset = label_iter->second;
            auto label_docids = (*query_docids)[label_iter->first];
            for (int doc_idx = 0; doc_idx < label_docids->size(); doc_idx++) {
                feature_vector *fv = (*label_dataset)[doc_idx];
                string docid = (*label_docids)[doc_idx];
                double score = model->predict(*fv);
                doc_scores->push_back(std::make_pair(docid, score));
            }
        }
        std::sort(doc_scores->begin(), doc_scores->end(), compare_docscore);
        cout << "qid: " << query_iter->first << endl;
        for (auto score_iter = doc_scores->begin(); score_iter != doc_scores->end(); score_iter++) {
            cout << "docid: " << (*score_iter).first << ", score: " << (*score_iter).second << endl;
        }
        delete doc_scores;
    }
    delete qids;
    free_dataset(dataset);
    free_docids(docids);
    free_relevances(relevance_map);
}

void free_relevances(unordered_map<int, unordered_map<string, int>*> *relevances) {
    for (auto query_iter = relevances->begin(); query_iter != relevances->end(); query_iter++) {
        auto query_relevances = query_iter->second;
        delete query_relevances;
    }
    delete relevances;
}

void free_docids(unordered_map<int, unordered_map<int, vector<string>*>*> *docids) {
    for (auto query_iter = docids->begin(); query_iter != docids->end(); query_iter++) {
        auto query_docids = query_iter->second;
        for (auto label_iter = query_docids->begin(); label_iter != query_docids->end(); label_iter++) {
            auto label_docids = label_iter->second;
            delete label_docids;
        }
        delete query_docids;
    }
    delete docids;
}

/**
 * Train the pairwise ranker model
 * @return
 */
int train(string data_dir, sgd_model *model) {
    vector<int> *training_qids = new vector<int>();
    unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *training_dataset
            = new unordered_map<int, unordered_map<int, vector<feature_vector*>*>*>();
    read_data(TRAINING, data_dir, training_qids, training_dataset, nullptr, nullptr);
    int n_iter = 100000;

    std::unique_ptr<learn::loss::loss_function> loss
            = learn::loss::make_loss_function(learn::loss::hinge::id.to_string());

    for (int i = 0; i < n_iter; i++) {
        int random_seed = i;
        std::pair<tupl, tupl> data_pair = getRandomPair(training_qids, training_dataset, random_seed);
        feature_vector a, b;
        int y_a, y_b, qid;
        std::tie(a, y_a, qid) = data_pair.first;
        std::tie(b, y_b, qid) = data_pair.second;
        feature_vector x = a;
        x -= b;
        double expected_label = y_a - y_b;
        double los = model->train_one(x, expected_label, *loss);
        if (los!=0) {
            cout<<"Loss :"<<los<<endl;
        }

    }
    loss.reset();
    delete training_qids;
    free_dataset(training_dataset);
    return 0;
}

void free_dataset(unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *dataset) {
    for (auto qid_iter = dataset->begin(); qid_iter != dataset->end(); qid_iter++) {
        auto query_dataset = qid_iter->second;
        for (auto label_iter = query_dataset->begin(); label_iter != query_dataset->end(); label_iter++) {
            auto label_dataset = label_iter->second;
            for (auto features_iter = label_dataset->begin(); features_iter != label_dataset->end(); features_iter++) {
                delete *features_iter;
            }
            delete label_dataset;
        }
        delete query_dataset;
    }
    delete dataset;
}

/**
 * Return a random pair of tuple for training the svm classifier
 * Tuple is of type (feature_vec, label, qid)
 * @param indexed_dataset
 * @return
 */
std::pair<tupl, tupl> getRandomPair(vector<int> *training_qids,
                                    unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *training_dataset,
                                    int random_seed) {

    std::default_random_engine generator(random_seed);
    int rel_levels = 0;
    unordered_map<int, vector<feature_vector*>*> *qid_vec;
    int qid = 0;
    do {
        //select q uniformly at random from Q
        int max_q = training_qids->size();
        std::uniform_int_distribution<int> qid_distribution(0, max_q-1);
        int q_index = qid_distribution(generator);
        qid = (*training_qids)[q_index];
        qid_vec = (*training_dataset)[qid];
        rel_levels = qid_vec->size();

    } while(rel_levels <= 1);

    //select ya uniformly at random from Y [q]
    int max_ya = qid_vec->size();
    std::uniform_int_distribution<int> ya_distribution(0, max_ya -1);
    int ya_index = ya_distribution(generator);

    int count = 0;
    int ya = ya_index;
    for (auto iter = qid_vec->begin(); iter != qid_vec->end() && count <= ya; iter++, count++) {
        ya_index = iter->first;
    }

    //select (a, ya, q) uniformly at random from P[q][ya]
    int max_a = (*qid_vec)[ya_index]->size();
    std::uniform_int_distribution<int> a_distribution(0, max_a-1);
    int a_index = a_distribution(generator);
    feature_vector a = *((*(*qid_vec)[ya_index])[a_index]);
    tupl d1  = std::make_tuple(a, ya_index, qid);

    auto save = (*qid_vec)[ya_index];
    qid_vec->erase(ya_index);

    //select yb uniformly at random from Y [q] − ya.
    int max_yb = max_ya -1;
    std::uniform_int_distribution<int> yb_distribution(0, max_yb-1);
    int yb_index = yb_distribution(generator);
    count = 0;
    int yb = yb_index;
    for (auto iter = qid_vec->begin(); iter != qid_vec->end() && count <= yb; iter++, count++) {
        yb_index = iter->first;
    }

    (*qid_vec)[ya_index] = save;

    //select (b, yb, q) uniformly at random from P[q][yb]
    int max_b = (*qid_vec)[yb_index]->size();
    std::uniform_int_distribution<int> b_distribution(0, max_b-1);
    int b_index = b_distribution(generator);
    feature_vector b = *((*(*qid_vec)[yb_index])[b_index]);
    tupl d2  = std::make_tuple(b, yb_index, qid);

    return std::make_pair(d1,d2);
}

/**
 * Read data from dataset and store it as nested hash-tables
 * @param data_type
 * @param data_dir
 * @param qids
 * @param dataset
 */
void read_data(DATA_TYPE data_type, string data_dir, vector<int> *qids,
               unordered_map<int, unordered_map<int, vector<feature_vector*>*>*> *dataset,
                unordered_map<int, unordered_map<int, vector<string>*>*> *docids,
                unordered_map<int, unordered_map<string, int>*> *relevance_map) {
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

        unordered_map<int, vector<feature_vector*> *> *query_dataset;
        if (dataset->find(qid) != dataset->end()) {
            query_dataset = (*dataset)[qid];
        } else {
            qids->push_back(qid);
            query_dataset = new unordered_map<int, vector<feature_vector*>*>();
            (*dataset)[qid] = query_dataset;
        }
        vector<feature_vector*> *label_dataset;
        if (query_dataset->find(label) != query_dataset->end()) {
            label_dataset = (*query_dataset)[label];
        } else {
            label_dataset = new vector<feature_vector*>();
            (*query_dataset)[label] = label_dataset;
        }
        feature_vector *features = new feature_vector(0);
        for (feature_idx = 0; feature_idx < 46; feature_idx++) {
            iss >> tmp_str;
            stringstream ssid(tmp_str.substr(0, tmp_str.find(':')));
            ssid >> feature_id;
            stringstream ssval(tmp_str.substr(tmp_str.find(':') + 1));
            ssval >> feature_val;
            (*features)[term_id{feature_id - 1}] = feature_val;
        }
        label_dataset->push_back(features);
        iss >> tmp_str;
        iss >> tmp_str;
        iss >> docid;
        if (data_type != TRAINING) {
            unordered_map<int, vector<string> *> *query_docids;
            if (docids->find(qid) != docids->end()) {
                query_docids = (*docids)[qid];
            } else {
                query_docids = new unordered_map<int, vector<string>*>();
                (*docids)[qid] = query_docids;
            }
            vector<string> *label_docids;
            if (query_docids->find(label) != query_docids->end()) {
                label_docids = (*query_docids)[label];
            } else {
                label_docids = new vector<string>();
                (*query_docids)[label] = label_docids;
            }
            label_docids->push_back(docid);
            unordered_map<string, int> *doc_relevance;
            if (relevance_map->find(qid) != relevance_map->end()) {
                doc_relevance = (*relevance_map)[qid];
            } else {
                doc_relevance = new unordered_map<string, int>();
                (*relevance_map)[qid] = doc_relevance;
            }
            (*doc_relevance)[docid] = label;
        }
    }
}