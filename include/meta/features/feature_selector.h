/**
 * @file feature_selector.h
 * @author Sean Massung
 * @author Siddharth Shukramani
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FEATURE_SELECTOR_H_
#define META_FEATURE_SELECTOR_H_

#include <memory>
#include <string>
#include <vector>

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/index/disk_index.h"
#include "meta/io/filesystem.h"
#include "meta/learn/instance.h"
#include "meta/stats/multinomial.h"
#include "meta/succinct/sarray.h"
#include "meta/util/progress.h"
#include "meta/util/sparse_vector.h"

namespace meta
{
namespace features
{
/**
 * The base class that shows the feature selection interface for MeTA, allowing
 * dimensionality reduction for documents as well as descriptions of classes by
 * their useful features.
 *
 * Required config parameters:
 * ~~~toml
 * method = "corr-coef" # choose the feature selection algorithm
 * prefix = "folder-prefix"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * features-per-class = 20 # default
 * ~~~
 */
class feature_selector
{
  public:
    /**
     * @param prefix
     * @param total_labels
     * @param total_features
     */
    feature_selector(const std::string& prefix, uint64_t total_labels,
                     uint64_t total_features);

    /**
     * Default destructor.
     */
    virtual ~feature_selector() = default;

    /**
     * Prints a summary of the top k features for each class.
     * @param idx
     * @param k
     */
    virtual void print_summary(std::shared_ptr<index::disk_index> idx,
                               uint64_t k = 20) const;

    /**
     * @param term
     * @return whether the given term is currently "selected"
     */
    virtual bool selected(term_id term) const;

    /**
     * @return the total number of currently selected features
     */
    uint64_t total_selected() const;

    /**
     * Determines the new, condensed feature_id for a given term_id after
     * feature selection has been performed. This is only defined for
     * term_ids where `selected(term) == true`.
     * @param term
     * @return the new feature_id for this term after feature selection
     */
    learn::feature_id new_id(term_id term) const;

    /**
     * Determines the original term_id for a given feature_id after
     * feature selection has been performed. This is only defined for
     * feature_ids obtained via calling new_id(term).
     * @param feature
     * @return the original term_id for this feature
     */
    term_id old_id(learn::feature_id feature) const;

    /**
     * Sets the top k features for *each class* to be "selected"
     * @param k
     */
    virtual void select(uint64_t k = 20);

    /**
     * Selects approximately the top p percent features for the entire dataset,
     * \f$ p\in[0,1] \f$. Each class will then have \f$\frac{p\cdot |V|}{
     * |L|} \f$ features selected, where \f$|L|\f$ is the number of classes.
     * @param p
     */
    virtual void select_percent(double p = 0.05);

  protected:
    /**
     * Scores a (label, term) pair in the index according to the derived class's
     * feature selection method
     * @param lbl
     * @param tid
     */
    virtual double score(const class_label& lbl, term_id tid) const = 0;

    /**
     * @param tid
     * @return the probability of a specific term in the index
     */
    double prob_term(term_id tid) const;

    /**
     * @param lbl
     * @return the probability of a specific class in the index
     */
    double prob_class(const class_label& lbl) const;

    /**
     * Probability of term occuring in class
     * \f$ P(t, c) = \frac{c(t, c)}{T} \f$
     * @param tid
     * @param lbl
     * @return P(t, c)
     */
    double term_and_class(term_id tid, const class_label& lbl) const;

    /**
     * Probability of not seeing a term and a class:
     * \f$ P(t', c) = P(c) - P(t, c) \f$
     * @param tid
     * @param lbl
     * @return P(t', c)
     */
    double not_term_and_class(term_id tid, const class_label& lbl) const;

    /**
     * Probability of term not occuring in a class:
     * \f$ P(t, c') = P(t) - P(t, c) \f$
     * @param tid
     * @param lbl
     * @return P(t, c')
     */
    double term_and_not_class(term_id tid, const class_label& lbl) const;

    /**
     * Probability not in class c in which term t does not occur:
     * \f$ P(t', c') = 1 - P(t, c) - P(t', c) - P(t, c') \f$
     * @param tid
     * @param lbl
     * @return P(t', c')
     */
    double not_term_and_not_class(term_id tid, const class_label& lbl) const;

  private:
    /**
     * The internal implementation of a feature_selector object is an
     * sarray (with rank and select data structures) and a collection of
     * binary files. The sarray allows constant-time access to look up
     * a term_id and check whether it has been "selected". The binary files
     * are sorted by feature score for easy summary operations as well as
     * changing which top features are set to be selected.
     *
     * This base feature_selector class calculates and contains four
     * distributions which may be used to calculate different feature
     * selection scores, implemented as derived classes.
     */

    /**
     * Creates the state of this feature_selector if necessary; this logic is
     * outside the constructor since it requires pure virtual functions
     * implemented by deriving classes.
     *
     * @param docs
     * @param features_per_class
     */
    template <class LabeledDatasetContainer>
    void init(const LabeledDatasetContainer& docs, uint64_t features_per_class)
    {
        // if the first class distribution exists, we have already created the
        // data for this feature_selector previously
        if (filesystem::file_exists(prefix_ + "/1.bin"))
            return;

        term_prob_.clear();
        class_prob_.clear();
        co_occur_.clear();

        calc_probs(docs);
        score_all();
        select(features_per_class);
    }

    /// friend the factory function used to create feature_selectors, since
    /// they need to call the init
    template <class LabeledDatasetContainer>
    friend std::unique_ptr<feature_selector>
    make_selector(const cpptoml::table& config,
                  const LabeledDatasetContainer& docs);

    /**
     * Calculates the probabilities of terms and classes given the current
     * index.
     */
    template <class LabeledDatasetContainer>
    void calc_probs(const LabeledDatasetContainer& docs)
    {
        uint64_t num_processed = 0;

        printing::progress prog{" > Calculating feature probs: ", docs.size()};

        for (const auto& instance : docs)
        {
            std::stringstream ss;
            ss << docs.label(instance);
            class_label lbl{ss.str()};

            class_prob_.increment(lbl, 1);

            for (const auto& count : instance.weights)
            {
                term_id tid{count.first};

                term_prob_.increment(tid, count.second);
                co_occur_.increment(std::make_pair(lbl, tid), count.second);
            }

            prog(++num_processed);
        }

        prog.end();
    }

    /**
     * Calculates the feature score for each (label, term) pair.
     */
    void score_all();

    /// Where the feature selection data is stored
    const std::string prefix_;

    /// Total number of labels
    uint64_t total_labels_;

    /// Total number of features
    uint64_t total_features_;

    /// Storage for rank/select queries on selected feature_ids
    util::optional<succinct::sarray> sarray_;

    /// Query structure for rank queries
    util::optional<succinct::sarray_rank> s_rank_;

    /// Query structure for select queries
    util::optional<succinct::sarray_select> s_select_;

    /// P(t) in the entire collection, indexed by term_id
    stats::multinomial<term_id> term_prob_;

    /// P(c) in the collection, indexed by class_label
    stats::multinomial<class_label> class_prob_;

    /// P(c,t) indexed by [class_label][term_id]
    stats::multinomial<std::pair<class_label, term_id>> co_occur_;
};

/**
 * Basic exception for feature selectors.
 */
class feature_selector_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * Filters a labeled_dataset to contain only features that are marked as
 * selected
 * by a given feature selector.
 *
 * @param dataset The dataset to filter
 * @param selector The feature selector to use
 */
template <class Dataset>
Dataset filter_dataset(const Dataset& dataset, const feature_selector& selector)
{
    return Dataset(
        dataset.begin(), dataset.end(), selector.total_selected(),
        [&](const learn::instance& instance) {
            auto weights = instance.weights;

            using pair_type = learn::feature_vector::pair_type;

            weights.erase(std::remove_if(weights.begin(), weights.end(),
                                         [&](const pair_type& weight) {
                                             return !selector.selected(
                                                 weight.first);
                                         }),
                          weights.end());
            weights.shrink_to_fit();
            std::transform(weights.begin(), weights.end(), weights.begin(),
                           [&](const pair_type& weight) {
                               return pair_type{selector.new_id(weight.first),
                                                weight.second};
                           });
            return weights;
        },
        [&](const learn::instance& instance) {
            return dataset.label(instance);
        });
}
}
}
#endif
