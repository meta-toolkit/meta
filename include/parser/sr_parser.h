/**
 * @file sr_parser.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_SR_PARSER_H_
#define META_PARSER_SR_PARSER_H_

#include <random>

#include "meta.h"
#include "util/optional.h"
#include "util/persistent_stack.h"
#include "util/sparse_vector.h"
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

    struct parser_state
    {
        parser_state(const parse_tree& tree);

        void advance(transition trans);

        util::persistent_stack<std::unique_ptr<node>> stack;
        std::vector<std::unique_ptr<leaf_node>> queue;
        size_t q_idx;
        bool done;
    };

    struct training_data
    {
        training_data(training_options options, std::vector<parse_tree>& trees);

        void shuffle();
        size_t size() const;
        const parse_tree& tree(size_t idx) const;
        const std::vector<transition>& transitions(size_t idx) const;

        training_options options;

        std::vector<parse_tree>& trees;
        std::vector<std::vector<transition>> all_transitions;

        std::vector<size_t> indices;

        std::default_random_engine rng;
    };

    struct training_batch
    {
        training_data& data;
        size_t start;
        size_t end;
    };

    using weight_vector = util::sparse_vector<std::string, double>;
    using weight_vectors = util::sparse_vector<transition, weight_vector>;

    void load(const std::string& prefix);

    weight_vectors train_batch(training_batch batch);

    transition best_transition(const weight_vector& features) const;

    weight_vector featurize(const parser_state& state) const;

    void unigram_featurize(const parser_state& state,
                           weight_vector& feats) const;

    void bigram_featurize(const parser_state& state,
                          weight_vector& feats) const;

    void trigram_featurize(const parser_state& state,
                           weight_vector& feats) const;

    void children_featurize(const parser_state& state,
                            weight_vector& feats) const;

    void unigram_stack_feats(const node* n, std::string prefix,
                             weight_vector& feats) const;

    void child_feats(const node* n, std::string prefix, weight_vector& feats,
                     bool doubs) const;

    /// Storage for the weights for each possible transition
    weight_vectors weights_;
};
}
}
#endif
