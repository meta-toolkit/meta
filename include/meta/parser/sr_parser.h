/**
 * @file sr_parser.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_SR_PARSER_H_
#define META_PARSER_SR_PARSER_H_

#include <map>
#include <random>
#include <unordered_map>

#include "meta/classify/models/linear_model.h"
#include "meta/config.h"
#include "meta/meta.h"
#include "meta/parallel/thread_pool.h"
#include "meta/parser/transition_map.h"
#include "meta/parser/trees/parse_tree.h"
#include "meta/sequence/sequence.h"
#include "meta/util/optional.h"
#include "meta/util/sparse_vector.h"

namespace meta
{
namespace parser
{

class parse_tree;
class state;

/**
 * A shift-reduce constituency parser. The model is a simple linear
 * classifier learned via the generalized averaged perceptron algorithm
 * that seeks to classify a parser action given a parser state.
 *
 * @see http://people.sutd.edu.sg/~yue_zhang/pub/acl13.muhua.pdf
 * @see http://www.aclweb.org/anthology/W09-3825
 */
class sr_parser
{
  public:
    /**
     * The set of training algorithms available for the parser.
     */
    enum class training_algorithm
    {
        EARLY_TERMINATION,
        BEAM_SEARCH
    };

    /**
     * Training options required for learning a parser model.
     */
    struct training_options
    {
        /**
         * How many trees should be put together into a single batch for
         * learning?
         */
        uint64_t batch_size = 25;

        /**
         * How many states should be kept on the beam? (valid for beam
         * search only)
         */
        uint64_t beam_size = 8;

        /**
         * How many iterations to run the training algorithm for?
         */
        uint64_t max_iterations = 40;

        /**
         * The seed for the random number generator used for shuffling
         * examples during training.
         */
        std::random_device::result_type seed = std::random_device{}();

        /**
         * How many threads to use for training.
         */
        uint64_t num_threads = std::thread::hardware_concurrency();

        /**
         * The algorithm to use for training. Defaults to
         * training_algorithm::EARLY_TERMINATION, which is a greedy
         * training method that results in small(-er) models.
         */
        training_algorithm algorithm = training_algorithm::EARLY_TERMINATION;

        /**
         * Default constructor.
         */
        training_options() = default;

        /**
         * Copy constructor.
         */
        training_options(const training_options&) = default;
    };

    /**
     * Default constructor.
     */
    sr_parser() = default;

    /**
     * Loads a pre-trained parser from a prefix.
     *
     * @param prefix The prefix to load the parser model from
     */
    sr_parser(const std::string& prefix);

    /**
     * Parses a POS-tagged sentence (represented as a sequence::sequence).
     *
     * @param sentence The sentence to be tagged
     * @return the parse tree corresponding to the input sentence
     */
    parse_tree parse(const sequence::sequence& sentence) const;

    /**
     * Trains a model on the given parse trees using the supplied training
     * options.
     *
     * @param trees The full parse trees for training
     * @param options The options used for training
     */
    void train(std::vector<parse_tree>& trees, training_options options);

    /**
     * @param prefix The prefix to store the model in
     */
    void save(const std::string& prefix) const;

    /**
     * Sparse vector representation of a state's features.
     */
    using feature_vector = std::unordered_map<std::string, float>;

    /**
     * A single weight vector for a specific transition.
     */
    using weight_vector = util::sparse_vector<trans_id, float>;

    /**
     * A collection of weight vectors by feature type.
     */
    using weight_vectors = std::unordered_map<std::string, weight_vector>;

  private:
    /**
     * The training data for the parser.
     */
    class training_data;

    /**
     * A training batch.
     */
    struct training_batch
    {
        training_data& data;
        size_t start;
        size_t end;
    };

    /**
     * Analyzer responsible for converting a parser state to a
     * feature_vector.
     */
    class state_analyzer;

    /**
     * @param prefix The prefix to load the model from
     */
    void load(const std::string& prefix);

    /**
     * Calculates a weight update on a given batch of training trees.
     *
     * @param batch The batch to learn on
     * @param pool The thread pool to use for parsing the batch in parallel
     * @param options The training options
     * @return a 3-tuple (update, correct actions, incorrect actions)
     */
    std::tuple<weight_vectors, uint64_t, uint64_t>
    train_batch(training_batch batch, parallel::thread_pool& pool,
                const training_options& options);

    /**
     * Calculates a weight update on a single tree.
     *
     * @param tree The training tree
     * @param transitions The correct transitions for parsing this tree
     * @param options The training options
     * @param update The weight vector to place the update in
     * @return (correct actions, incorrect actions)
     */
    std::pair<uint64_t, uint64_t> train_instance(
        const parse_tree& tree, const std::vector<trans_id>& transitions,
        const training_options& options, weight_vectors& update) const;

    /**
     * Calculates a weight update on a single tree, using the greedy early
     * termination training strategy.
     *
     * @param tree The training tree
     * @param transitions The correct transitions for parsing this tree
     * @param options The training options
     * @param update The weight vector to place the update in
     * @return (correct actions, incorrect actions)
     */
    std::pair<uint64_t, uint64_t>
    train_early_termination(const parse_tree& tree,
                            const std::vector<trans_id>& transitions,
                            weight_vectors& update) const;

    /**
     * Calculates a weight update on a single tree, using beam search.
     *
     * @param tree The training tree
     * @param transitions The correct transitions for parsing this tree
     * @param options The training options
     * @param update The weight vector to place the update in
     * @return (correct actions, incorrect actions)
     */
    std::pair<uint64_t, uint64_t> train_beam_search(
        const parse_tree& tree, const std::vector<trans_id>& transitions,
        const training_options& options, weight_vectors& update) const;

    /**
     * Computes the most likely transition according to the current model
     *
     * @param features The feature vector representation for the current
     * state
     * @param state The current state
     * @param check_legality Whether or not to limit the transitions to
     * only those that are "legal" according to the constraints given for
     * each transition
     */
    trans_id best_transition(const feature_vector& features, const state& state,
                             bool check_legality = false) const;

    using scored_trans = std::pair<trans_id, float>;

    /**
     * Computes the \f$k\f$ most likely transitions according to the
     * current model.
     * @param features The feature vector representation for the current
     * state
     * @param state The current state
     * @param check_legality Whether or not to limit the transitions to
     * only those that are "legal" according to the constraints given for
     * each transition
     */
    std::vector<scored_trans>
    best_transitions(const feature_vector& features, const state& state,
                     size_t num, bool check_legality = false) const;

    /**
     * Storage for the ids for each transition
     */
    transition_map trans_;

    /**
     * Storage for the weights for each possible transition
     */
    classify::linear_model<std::string, float, trans_id> model_;

    /**
     * Beam size used during training.
     */
    uint64_t beam_size_ = 1;
};

/**
 * Exception thrown during parser actions
 */
class sr_parser_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
#endif
