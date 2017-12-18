/**
 * @file pairwise_letor.h
 * @author Mihika Dave, Anthony Huang, Rachneet Kaur
 * @date 12/18/17
 */

#ifndef META_PAIRWISE_LETOR_H
#define META_PAIRWISE_LETOR_H

#include <functional>
#include <iostream>
#include <vector>
#include <random>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>
#include <chrono>
#include <algorithm>

#include "meta/learn/loss/all.h"
#include "meta/learn/loss/hinge.h"
#include "meta/learn/loss/loss_function.h"
#include "meta/learn/loss/loss_function_factory.h"
#include "meta/learn/sgd.h"
#include "meta/learn/instance.h"
#include "meta/learn/dataset.h"
#include "meta/classify/classifier/svm_wrapper.h"
#include "meta/classify/classifier/classifier.h"


using namespace std;
using namespace meta::util;
using namespace meta::classify;

namespace meta
{
namespace learn
{
namespace learntorank
{
/**
 * This class implements pairwise learning to rank with the help of binary classifiers.
 * The ranker here mainly follows the Stochastic Pairwise Descent algorithm based on
 * D. Sculley's paper on 'Large Scale Learning to Rank'.
 *
 * @see https://static.googleusercontent.com/media/research.google.com/en//pubs/archive/35662.pdf
 */
class pairwise_letor {
    public:
        using tupl = std::tuple<feature_vector, int, string>;

        enum DATA_TYPE {
            TRAINING,
            VALIDATION,
            TESTING
        };

        enum CLASSIFY_TYPE {
            LIBSVM,
            SPD,
        };

        typedef struct forwardnode {
            operator int() const {
                return label;
            }
            operator feature_vector() const {
                return fv;
            }
            int label;
            feature_vector fv;
        } forward_node;

    public:

        /**
         * Train the pairwise ranker model
         * @param data_dir
         * @param feature_nums
         * @param model
         */
        void train(string data_dir, int feature_nums, sgd_model *model);

        /**
         * Train the svm with a pair of data samples
         * @param data_dir
         * @param feature_nums
         * @param svm_path
         * @return
         */
        svm_wrapper *train_svm(string data_dir, int feature_nums, string svm_path);

        /**
         * Validate the learnt model
         * @param data_dir
         * @param feature_nums
         * @param classify_type
         * @param wrapper
         * @param model
         */
        void validate(string data_dir,
                      int feature_nums,
                      CLASSIFY_TYPE classify_type,
                      svm_wrapper *wrapper,
                      sgd_model *model);

        /**
         * Test the model on testing dataset
         * @param data_dir
         * @param feature_nums
         * @param classify_type
         * @param wrapper
         * @param model
         */
        void test(string data_dir,
                  int feature_nums,
                  CLASSIFY_TYPE classify_type,
                  svm_wrapper *wrapper,
                  sgd_model *model);

    private:

        /**
         * Read data from the dataset and store it as nested hash-tables
         * @param data_type
         * @param data_dir
         * @param qids
         * @param dataset
         * @param docids
         * @param relevance_map
         * @param feature_nums
         */
        void read_data(DATA_TYPE data_type,
                       string data_dir,
                       vector<string> *qids,
                       unordered_map<string, unordered_map<int, vector<feature_vector>>> *dataset,
                       unordered_map<string, unordered_map<int, vector<string>>> *docids,
                       unordered_map<string, unordered_map<string, int>> *relevance_map,
                       int feature_nums);

        /**
         * Return a random pair of tuple for training the svm classifier
         * Tuple is of type (feature_vec, label, qid)
         * @param training_qids
         * @param training_dataset
         * @param random_seed
         * @return
         */
        std::pair<tupl, tupl> getRandomPair(vector<string> *training_qids, unordered_map<string,
                unordered_map<int, vector<feature_vector>>> *training_dataset, int random_seed);

        /**
         * Build nodes from dataset
         * @param training_dataset
         * @param dataset_nodes
         */
        void build_dataset_nodes(unordered_map<string, unordered_map<int, vector<feature_vector>>> *training_dataset,
                            vector<forward_node> *dataset_nodes);

        /**
         * Compare the relative rank between the 2 data samples
         * @param p1
         * @param p2
         * @return
         */
        static bool compare_docscore(const pair<string, double> &p1, const pair<string, double> &p2) {
            return p1.second > p2.second;
        }

        /**
         * Compute the DCG
         * @param limit
         * @param rankings
         * @return
         */
        double compute_dcg(int limit, vector<int> &rankings);

        /**
         * Evaluate the dataset for precision, mean average precision, NDCG
         * @param qids
         * @param dataset
         * @param docids
         * @param relevance_map
         * @param feature_nums
         * @param classify_type
         * @param wrapper
         * @param model
         */
        void evaluate(vector<string> *qids,
                 unordered_map<string, unordered_map<int, vector<feature_vector>>> *dataset,
                 unordered_map<string, unordered_map<int, vector<string>>> *docids,
                 unordered_map<string, unordered_map<string, int>> *relevance_map,
                 int feature_nums,
                 CLASSIFY_TYPE classify_type,
                 svm_wrapper *wrapper,
                 sgd_model *model);

};

}
}
}
#endif
