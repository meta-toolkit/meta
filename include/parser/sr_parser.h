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

#include "meta.h"
#include "util/optional.h"
#include "util/persistent_stack.h"
#include "util/sparse_vector.h"
#include "parallel/parallel_for.h"
#include "parser/trees/parse_tree.h"
#include "parser/transition.h"

namespace meta
{
namespace parser
{

/**
 * A shift-reduce constituency parser.
 *
 * @see http://people.sutd.edu.sg/~yue_zhang/pub/acl13.muhua.pdf
 * @see http://www.aclweb.org/anthology/W09-3825
 */
class sr_parser
{
  public:
    enum class training_algorithm
    {
        EARLY_TERMINATION
    };

    struct training_options
    {
        uint64_t batch_size = 25;
        uint64_t beam_size = 8;
        uint64_t max_iterations = 40;
        uint64_t seed = std::random_device{}();
        uint64_t num_threads = std::thread::hardware_concurrency();
        training_algorithm algorithm = training_algorithm::EARLY_TERMINATION;

        training_options() = default;
        training_options(const training_options&) = default;
    };

    sr_parser() = default;

    sr_parser(const std::string& prefix);

    void train(std::vector<parse_tree>& trees, training_options options);

    void save(const std::string& prefix) const;

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  private:
    class state;

    MAKE_NUMERIC_IDENTIFIER(trans_id, uint16_t)

    class transition_map
    {
      public:
        transition_map() = default;
        transition_map(const std::string& prefix);

        const transition& at(trans_id id) const;
        trans_id at(const transition& trans) const;

        transition& operator[](trans_id id);
        trans_id operator[](const transition& trans);

        void save(const std::string& prefix) const;

        uint64_t size() const;

      private:
        util::sparse_vector<transition, trans_id> map_;
        std::vector<transition> transitions_;
    };

    struct training_data
    {
        training_data(training_options options, std::vector<parse_tree>& trees);

        transition_map preprocess();

        void shuffle();
        size_t size() const;
        const parse_tree& tree(size_t idx) const;
        const std::vector<trans_id>& transitions(size_t idx) const;

        training_options options;

        std::vector<parse_tree>& trees;
        std::vector<std::vector<trans_id>> all_transitions;

        std::vector<size_t> indices;

        std::default_random_engine rng;
    };

    struct training_batch
    {
        training_data& data;
        size_t start;
        size_t end;
    };

    using feature_vector = std::unordered_map<std::string, float>;

    using weight_vector = util::sparse_vector<trans_id, float>;
    using weight_vectors = std::unordered_map<std::string, weight_vector>;

    class state_analyzer;

    void load(const std::string& prefix);

    weight_vectors train_batch(training_batch batch,
                               parallel::thread_pool& pool);

    trans_id best_transition(const feature_vector& features) const;

    feature_vector featurize(const state& state) const;

    void condense();

    /// Storage for the ids for each transition
    transition_map trans_;

    /// Storage for the weights for each possible transition
    weight_vectors weights_;
};
}
}
#endif
