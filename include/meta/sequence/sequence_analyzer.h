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

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <vector>

#include "meta/config.h"
#include "meta/meta.h"
#include "meta/sequence/sequence.h"
#include "meta/util/invertible_map.h"

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
 * void operator()(const sequence& seq, uint64_t index, collector& coll)
 * ~~~
 *
 * and can only refer to the symbols in the sequence, *not* the tags! These
 * functions should not modify the sequence directly and should instead use
 * the `collector` interface. For example:
 *
 * ~~~{.cpp}
 * // feature function that gets the current word
 * auto fun = [](const sequence& seq, uint64_t t, collector& coll)
 * {
 *     std::string word = seq[t].symbol();
 *     coll.add("w[t]=" + word, 1);
 * };
 * ~~~
 */
class sequence_analyzer
{
  public:
    /**
     * Default constructor.
     */
    sequence_analyzer() = default;

    /**
     * Constructs a new sequence analyzer that will load its output from
     * the given prefix (folder).
     *
     * @param prefix The folder to load/save mappings to
     */
    sequence_analyzer(const std::string& prefix);

    /**
     * Sequence analyzers may be copy constructed.
     */
    sequence_analyzer(const sequence_analyzer&) = default;

    /**
     * Sequence analyzers may be move constructed.
     */
    sequence_analyzer(sequence_analyzer&&) = default;

    /**
     * Sequence analyzers may be copy assigned.
     */
    sequence_analyzer& operator=(const sequence_analyzer&) = default;

    /**
     * Sequence analyzers may be move assigned.
     */
    sequence_analyzer& operator=(sequence_analyzer&&) = default;

    /**
     * Loads a sequence analyzer from a folder given by prefix.
     * @param prefix the prefix to load the analyzer from
     */
    void load(const std::string& prefix);

    /**
     * Saves the sequence analyzer into the folder given by prefix.
     *
     * @param prefix The folder to save the analyzer to
     */
    void save(const std::string& prefix) const;

    /**
     * Analyzes a sequence, generating new label_ids and feature_ids for
     * unseen elements.
     *
     * @param sequence The sequence to be analyzed
     */
    void analyze(sequence& sequence);

    /**
     * Analyzes a single point in a sequence, generating new label_ids and
     * feature_ids for unseen elements.
     *
     * @param sequence The sequence to be analyzed
     * @param t The position in the sequence to be analyzed
     */
    void analyze(sequence& sequence, uint64_t idx);

    /**
     * Analyzes a sequence, but ignores any new label_ids or feature_ids.
     * Used for analyzing test items, for example, so that existing models
     * don't need to special case unseen feature ids.
     *
     * @param sequence The sequence to be analyzed
     */
    void analyze(sequence& sequence) const;

    /**
     * Analyzes a single point in a sequence,b ut ignores any new
     * label_ids or feature_ids. Used for analyzing test items, for
     * example, so that existing models don't need to special case unseen
     * feature ids.
     */
    void analyze(sequence& sequence, uint64_t idx) const;

    /**
     * Looks up the feature id for the given string representation. If one
     * doesn't exist, it will assign the next feature_id to this string
     *
     * @param feature The string representation of the feature
     * @return the feature id associated (or just assigned to) this feature
     */
    feature_id feature(const std::string& feature);

    /**
     * Looks up the feature_id for the given string representation. If one
     * doesn't exist, it will simply assign the next feature_id to the
     * string, but it will not remember the assignment.
     *
     * @param feature The string representation of the feature
     * @return the feature id associated with this feature, or the
     * "one-past-the-end" feature id
     */
    feature_id feature(const std::string& feature) const;

    /**
     * @return the number of feature_ids used so far to describe observations
     */
    uint64_t num_features() const;

    /**
     * @param lbl The tag
     * @return the label_id assigned a given tag
     */
    label_id label(tag_t lbl) const;

    /**
     * @param lbl The label_id
     * @return the tag that corresponds with this label_id
     */
    tag_t tag(label_id lbl) const;

    /**
     * @return the number of labels used so far to describe observations
     */
    uint64_t num_labels() const;

    /**
     * @return the prefix for this analyzers files
     */
    const std::string& prefix() const;

    /**
     * @return The invertible_map that stores the label id mapping
     */
    const util::invertible_map<tag_t, label_id>& labels() const;

    /**
     * Adds an observation function to the list of functions to be used
     * for analyzing observations.
     *
     * @param function The function to add
     */
    template <class Function>
    void add_observation_function(Function&& function)
    {
        obs_fns_.emplace_back(std::forward<Function>(function));
    }

    /**
     * Interface class used for analyzing observations inside
     * user-provided feature functions.
     */
    class collector
    {
      public:
        /**
         * Constructs the collector over a given observation.
         * @param obs A pointer to the observation to be analyzed
         */
        collector(observation* obs) : obs_{obs}
        {
            // nothing
        }

        /**
         * Writes all analyzed information out to the observation.
         */
        ~collector()
        {
            using pair = std::pair<feature_id, double>;
            std::sort(feats_.begin(), feats_.end(),
                      [](const pair& lhs, const pair& rhs) {
                          return lhs.first < rhs.first;
                      });

            obs_->features(std::move(feats_));
        }

        /**
         * Adds a new feature to this observation.
         * @param feat The string representation of the feature to add
         * @param amount The value associated with this feature (typically 1)
         */
        virtual void add(const std::string& feat, double amount)
        {
            feats_.emplace_back(feature(feat), amount);
        }

      protected:
        /**
         * @param feat The feature to obtain an id for
         * @return the feature_id for this feature
         */
        virtual feature_id feature(const std::string& feat) = 0;

        /// the observation we are collecting data for
        observation* obs_;
        /// the feature vector that will be placed into the observation
        observation::feature_vector feats_;
    };

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  private:
    /**
     * Implementation-detail collector.
     */
    template <class Analyzer>
    class basic_collector : public collector
    {
      public:
        /**
         * Creates the collector with the given analyzer.
         * @param analyzer The analyzer for this collector
         * @param obs The observation to be analyzed
         */
        basic_collector(Analyzer* analyzer, observation* obs)
            : collector{obs}, analyzer_{analyzer}
        {
            // nothing
        }

      protected:
        /// back-pointer to the analyzer for this collector
        Analyzer* analyzer_;

        virtual feature_id feature(const std::string& feat)
        {
            return analyzer_->feature(feat);
        }
    };

  public:
    /**
     * Non-const version of the collector.
     */
    class default_collector : public basic_collector<sequence_analyzer>
    {
      public:
        using basic_collector<sequence_analyzer>::basic_collector;
    };

    /**
     * Const version of the collector. This collector will only generate
     * existing feature_ids and label_ids.
     */
    class const_collector : public basic_collector<const sequence_analyzer>
    {
      public:
        using basic_collector<const sequence_analyzer>::basic_collector;

        // special case add to not actually add if a brand new feature id
        // is found
        virtual void add(const std::string& feat, double amount)
        {
            auto fid = feature(feat);
            if (fid != analyzer_->num_features())
                feats_.emplace_back(fid, amount);
        }
    };

  private:
    /**
     * Loads the feature_id mapping from disk.
     * @param prefix The folder to load the mapping from
     */
    void load_feature_id_mapping(const std::string& prefix);

    /**
     * Loads the label_id mapping from disk.
     * @param prefix The folder to load the mapping from
     */
    void load_label_id_mapping(const std::string& prefix);

    /**
     * Adds a feature to an observation.
     * @param obs The observation
     * @param feature The string representing the feature
     * @param weight The weight for the feature (default = 1.0)
     */
    void add_feature(observation& obs, const std::string& feature,
                     double weight = 1.0);

    /// The observation functions
    std::vector<std::function<void(const sequence&, uint64_t, collector&)>>
        obs_fns_;

    /// The feature_id mapping (string to id)
    std::unordered_map<std::string, feature_id> feature_id_mapping_;

    /// The label_id mapping (tag_t to label_id)
    util::invertible_map<tag_t, label_id> label_id_mapping_;
};

/**
 * Constructs a sequence_analyzer that is specialized for part-of-speech
 * tagging. Uses a predefined set of observation functions.
 */
sequence_analyzer default_pos_analyzer();
}
}
#endif
