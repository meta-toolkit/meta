#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <unordered_map>


#include "meta/learn/loss/all.h"
#include "meta/learn/loss/hinge.h"
#include "meta/learn/loss/loss_function.h"
#include "meta/learn/loss/loss_function_factory.h"
#include "meta/learn/sgd.h"
#include "../../../tests/farm_hash_test.h"

using namespace meta;
using namespace learn;
using tupl = std::tuple<feature_vector, int, int>;
using namespace std;
//using feature_id = term_id;
//using feature_vector = util::sparse_vector<feature_id, double>;

int main(int argc, char* argv[])
{

    std::cerr << "Hello LETOR!" << std::endl;

    std::unique_ptr<learn::loss::loss_function> loss = learn::loss::make_loss_function(learn::loss::hinge::id.to_string());
    learn::sgd_model *model = new learn::sgd_model(46);
    std::cerr << "sgd model default settings: " << model->default_learning_rate << std::endl;
    std::cerr << model->default_l2_regularizer << std::endl;
    std::cerr << model->default_l1_regularizer << std::endl;

    std::cerr << "Bye LETOR!" << std::endl;

    return 0;
}

/**
 * Return a random pair of tuple for training the svm classifier
 * @param indexed_dataset
 * @return
 */
std::pair<tupl, tupl> getRandomPair(std::vector<int> indexed_dataset) {
    std::default_random_engine generator;

    //select q uniformly at random from Q
    int max_q = indexed_dataset.length();
    std::uniform_int_distribution<int> qid_distribution(0, max_q);
    int q_index = qid_distribution(generator);

    unordered_map<int, vector<feature_vector>>> qid_vec = indexed_dataset[q_index];
    int qid = qid_vec[]



    //select ya uniformly at random from Y [q]
    int max_ya = qid.length;
    std::uniform_int_distribution<int> ya_distribution(0, max_ya);
    int ya_index = ya_distribution(generator);

    //select (a, ya, q) uniformly at random from P[q][ya]
    int max_a = qid[ya_index].length;
    std::uniform_int_distribution<int> a_distribution(0, max_a);
    int a_index = a_distribution(generator);
    feature_vector a = qid[ya_index][a_index];

    tupl d1  = std::make_tuple(a, ya_index, qid);



    //select yb uniformly at random from Y [q] − ya.
    int max_yb = qid.length - 1;
    std::uniform_int_distribution<int> yb_distribution(0, max_yb);
    int yb_index = yb_distribution(generator);

    //select (b, yb, q) uniformly at random from P[q][yb]
    int max_b = qid[yb_index].length;
    std::uniform_int_distribution<int> b_distribution(0, max_b);
    int b_index = b_distribution(generator);
    feature_vector b = qid[yb_index][b_index];

    tupl d2  = std::make_tuple(b, yb_index, qid);

    return std::make_pair(d1,d2);
}


/**
 * Train the pairwise ranker model
 * @return
 */
int train(){
    std::vector<int> indexed_dataset = read_data();

    int n_iter = 1000000;

    learn::sgd_model *model = new learn::sgd_model(46);

    std::unique_ptr<learn::loss::loss_function> loss
            = learn::loss::make_loss_function(learn::loss::hinge::id.to_string());

    for (int i = 0; i <n_iter; i++) {
        std::pair<tupl, tupl> data_pair = getRandomPair(indexed_dataset);

        feature_vector a, b;
        int y_a, y_b, qid;

        std::tie(a, y_a, qid) = data_pair.first;

        std::tie(b, y_b, qid) = data_pair.second;

        feature_vector& x = a - b;

        double expected_label = data_pair.first.label - data_pair.second.label;

        model->train_one(x, expected_label, *loss);

    }

    return 0;
}