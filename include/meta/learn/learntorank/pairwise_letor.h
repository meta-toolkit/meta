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
 * This class implements pairwise learning to rank with binary classifiers.
 * The ranker here mainly follows the Stochastic Pairwise Descent algorithm
 * based on D. Sculley's paper on 'Large Scale Learning to Rank'.
 *
 * @see https://static.googleusercontent.com/media/research.google.com/en//
 * pubs/archive/35662.pdf
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
         * Constructor.
         * @param num_features The number of features for the pairwise model
         */
        pairwise_letor(size_t num_features);

        /**
         * Train the pairwise ranker model
         * @param data_dir Path to directory containing train.txt
         */
        void train(string data_dir);

        /**
         * Train the svm with a pair of data samples
         * @param data_dir Path to directory containing train.txt
         * @param svm_path The path to the liblinear/libsvm library
         */
        void train_svm(string data_dir, string svm_path);

        /**
         * Validate the learnt model
         * @param data_dir The path to the directory containing vali.txt
         * @param classify_type The classifier type to use
         */
        void validate(string data_dir,
                      int feature_nums,
                      CLASSIFY_TYPE classify_type);

        /**
         * Test the model on testing dataset
         * @param data_dir The path to the directory containing test.txt
         * @param classify_type The classifier type to use
         */
        void test(string data_dir,
                  CLASSIFY_TYPE classify_type);

    private:

        /// number of features for this letor model
        size_t num_features_;

        /// sgd_model for training and testing
        unique_ptr<sgd_model> model_;

        /// binary svm wrapper for training and testing
        unique_ptr<svm_wrapper> wrapper_;

        /**
         * Read data from the dataset and store it as nested hash-tables
         * @param data_type The type of data (train, vali, or test)
         * @param data_dir Path to directory containing train/vali/test.txt
         * @param qids Vector to store ids of queries
         * @param dataset Map to store nested data mapping: query => label => doc
         * @param docids Map to store docids in each query and label
         * @param relevance_map Map to store relevance of docs in each query
         */
        void read_data(DATA_TYPE data_type,
                       string data_dir,
                       vector<string>& qids,
                       unordered_map<string, unordered_map<int, vector<feature_vector>>>& dataset,
                       unique_ptr<unordered_map<string, unordered_map<int, vector<string>>>> docids,
                       unique_ptr<unordered_map<string, unordered_map<string, int>>> relevance_map);

        /**
         * Return a random pair of tuple for training the svm classifier
         * Tuple is of type (feature_vec, label, qid)
         * @param training_qids Vector holding ids of all queries
         * @param train_dataset Map holding nested data mapping: query => label => doc
         * @param random_seed The random seed used to randomly choose data
         * @return the random pair
         */
        pair<tupl, tupl> getRandomPair(
                vector<string>& training_qids,
                unordered_map<string,unordered_map<int,vector<feature_vector>>>& train_dataset,
                int random_seed);

        /**
         * Build nodes from dataset for training svm_wrapper
         * @param train_dataset Map holding nested data mapping: query => label => doc
         * @param dataset_nodes Vector holding data nodes for SVM training
         */
        void build_dataset_nodes(
                unordered_map<string,unordered_map<int,vector<feature_vector>>>& train_dataset,
                vector<forward_node>& dataset_nodes);

        /**
         * Compare the relative rank between the 2 data samples
         * @param p1 The first pair to compare
         * @param p2 The second pair to compare
         * @return whether p1 is ranked before p2
         */
        static bool compare_docscore(
                const pair<string, double> &p1, const pair<string, double> &p2) {
            return p1.second > p2.second;
        }

        /**
         * Compute the DCG
         * @param limit The number of positions to compute DCG
         * @param rankings Vector holding ranking at each position
         * @return computed DCG
         */
        double compute_dcg(int limit, vector<int> &rankings);

        /**
         * Evaluate the dataset for precision, mean average precision, NDCG
         * @param qids Vector holding id for queries
         * @param dataset Map holding nested data mapping: query => label => doc
         * @param docids Map holding doc ids for each query and label
         * @param relevance_map Map holding relevance of each doc for each query
         * @param classify_type The classifier used for this pairwise model
         */
        void evaluate(vector<string>& qids,
                 unordered_map<string, unordered_map<int, vector<feature_vector>>>& dataset,
                 unordered_map<string, unordered_map<int, vector<string>>>& docids,
                 unordered_map<string, unordered_map<string, int>>& relevance_map,
                 CLASSIFY_TYPE classify_type);

};

}
}
}
#endif
