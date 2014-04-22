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
#include <unordered_map>

#include "meta.h"
#include "sequence/sequence.h"
#include "util/invertible_map.h"

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
    sequence_analyzer(const std::string& prefix);

    sequence_analyzer(const sequence_analyzer&) = default;
    sequence_analyzer(sequence_analyzer&&) = default;
    sequence_analyzer& operator=(const sequence_analyzer&) = default;
    sequence_analyzer& operator=(sequence_analyzer&&) = default;

    ~sequence_analyzer();
    void save();

    void analyze(sequence& sequence);

    feature_id feature(const std::string& feature);
    uint64_t num_features() const;

    label_id label(tag_t lbl) const;
    tag_t label(label_id lbl) const;
    uint64_t num_labels() const;

    const std::string& prefix() const;

    const util::invertible_map<tag_t, label_id>& labels() const;

    template <class Function>
    void add_observation_function(Function&& function)
    {
        obs_fns_.emplace_back(std::forward<Function>(function));
    }

    class collector
    {
      public:
        collector(sequence_analyzer* analyzer, observation* obs);
        ~collector();
        void add(const std::string& feature, double amount);
      private:
        sequence_analyzer* analyzer_;
        observation* obs_;
        observation::feature_vector feats_;
    };

  private:
    void load_feature_id_mapping();
    void load_label_id_mapping();

    /**
     * Adds a feature to an observation.
     * @param obs The observation
     * @param feature The string representing the feature
     * @param weight The weight for the feature (default = 1.0)
     */
    void add_feature(observation& obs, const std::string& feature,
                     double weight = 1.0);

    /// The observation functions
    std::vector<std::function<void(sequence&, uint64_t, collector&)>> obs_fns_;

    /// The feature_id mapping (string to id)
    std::unordered_map<std::string, feature_id> feature_id_mapping_;

    /// The label_id mapping (tag_t to label_id)
    util::invertible_map<tag_t, label_id> label_id_mapping_;

    /// The prefix to write the analyzer files to
    const std::string prefix_;
};

/**
 * Constructs a sequence_analyzer that is specialized for part-of-speech
 * tagging. Uses a predefined set of observation functions.
 */
sequence_analyzer default_pos_analyzer(const std::string& filename);

}
}
#endif
