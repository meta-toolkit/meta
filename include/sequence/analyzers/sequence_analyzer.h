/**
 * @file sequence_analyzer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_SEQUENCE_ANALYZER_H_
#define META_SEQUENCE_SEQUENCE_ANALYZER_H_

#include <vector>
#include <functional>

#include "sequence/sequence.h"

namespace meta
{
namespace sequence
{

/**
 * Analyzer that operates over sequences, generating features based on a
 * set of "observation functions". Observation functions must have an
 * `operator()` of the form:
 *
 * ~~~{.cpp}
 * void operator()(sequence& seq, uint64_t index)
 * ~~~
 *
 * and can only refer to the symbols in the sequence, *not* the tags! These
 * functions should modify `seq` by calling `increment()` on it for the
 * appropriate feature it extracts. For example:
 *
 * ~~~{.cpp}
 * ~~~
 */
class sequence_analyzer
{
  public:
    /**
     * Fills in the features maps for the observations in this sequence
     * by calling each of the observation functions in turn on every time
     * step in the given sequence.
     *
     * @param sequence The sequence to find features for
     */
    void analyze(sequence& sequence) const;

    template <class Function>
    void add_observation_function(Function&& function)
    {
        obs_fns_.emplace_back(std::forward<Function>(function));
    }
  private:
    /**
     * Adds a feature to an observation.
     * @param obs The observation
     * @param feature The string representing the feature
     * @param weight The weight for the feature (default = 1.0)
     */
    void add_feature(observation& obs, const std::string& feature,
                     double weight = 1.0);

    /// The observation functions
    std::vector<std::function<void(sequence& seq, uint64_t index)>> obs_fns_;

    /// The feature_id mapping (string to id)
    std::unordered_map<std::string, feature_id> feature_id_mapping_;
};

/**
 * Constructs a sequence_analyzer that is specialized for part-of-speech
 * tagging. Uses a predefined set of observation functions.
 */
sequence_analyzer default_pos_analyzer();

}
}
#endif
