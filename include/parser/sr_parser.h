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
#include "util/sparse_vector.h"
#include "parallel/thread_pool.h"
#include "parser/trees/parse_tree.h"
#include "parser/transition_map.h"
#include "sequence/sequence.h"

namespace meta
{
namespace parser
{

class parse_tree;
class state;

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

    parse_tree parse(const sequence::sequence& sentence) const;

    void train(std::vector<parse_tree>& trees, training_options options);

    void save(const std::string& prefix) const;

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    using feature_vector = std::unordered_map<std::string, float>;

    using weight_vector = util::sparse_vector<trans_id, float>;
    using weight_vectors = std::unordered_map<std::string, weight_vector>;

  private:
    class training_data;

    struct training_batch
    {
        training_data& data;
        size_t start;
        size_t end;
    };

    class state_analyzer;

    void load(const std::string& prefix);

    std::tuple<weight_vectors, uint64_t, uint64_t>
        train_batch(training_batch batch, parallel::thread_pool& pool,
                    const training_options& options);

    std::pair<uint64_t, uint64_t> train_instance(
        const parse_tree& tree, const std::vector<trans_id>& transitions,
        const training_options& options, weight_vectors& update) const;

    std::pair<uint64_t, uint64_t>
        train_early_termination(const parse_tree& tree,
                                const std::vector<trans_id>& transitions,
                                weight_vectors& update) const;

    trans_id best_transition(const feature_vector& features, const state& state,
                             bool check_legality = false) const;

    /// Storage for the ids for each transition
    transition_map trans_;

    /// Storage for the weights for each possible transition
    weight_vectors weights_;
};
}
}
#endif
