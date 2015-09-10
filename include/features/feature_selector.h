/**
 * @file feature_selector.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FEATURE_SELECTOR_H_
#define META_FEATURE_SELECTOR_H_

#include <string>
#include <vector>
#include <memory>

#include "cpptoml.h"
#include "index/forward_index.h"
#include "util/disk_vector.h"

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
 * prefix = "file-prefix"
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
     * @param config
     * @param idx
     */
    feature_selector(const std::string& prefix,
                     std::shared_ptr<index::forward_index> idx);

    /**
     * Default destructor.
     */
    virtual ~feature_selector() = default;

    /**
     * Prints a summary of the top k features for each class.
     * @param k
     */
    virtual void print_summary(uint64_t k = 20) const;

    /**
     * @param term
     * @return whether the given term is currently "selected"
     */
    virtual bool selected(term_id term) const;

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
     * @param lid
     * @param tid
     */
    virtual double score(label_id lid, term_id tid) const = 0;

    /**
     * @return the probability of a specific term in the index
     */
    double prob_term(term_id id) const;

    /**
     * @return the probability of a specific class in the index
     */
    double prob_class(label_id id) const;

    /**
     * Probability of term occuring in class
     * \f$ P(t, c) = \frac{c(t, c)}{T} \f$
     * @param term
     * @param label
     * @return P(t, c)
     */
    double term_and_class(term_id term, label_id label) const;

    /**
     * Probability of not seeing a term and a class:
     * \f$ P(t', c) = P(c) - P(t, c) \f$
     * @param term
     * @param label
     * @return P(t', c)
     */
    double not_term_and_class(term_id term, label_id label) const;

    /**
     * Probability of term not occuring in a class:
     * \f$ P(t, c') = P(t) - P(t, c) \f$
     * @param term
     * @param label
     * @return P(t, c')
     */
    double term_and_not_class(term_id term, label_id label) const;

    /**
     * Probability not in class c in which term t does not occur:
     * \f$ P(t', c') = 1 - P(t, c) - P(t', c) - P(t, c') \f$
     * @param term
     * @param label
     * @return P(t', c')
     */
    double not_term_and_not_class(term_id term, label_id label) const;

  private:
    /**
     * The internal implementation of a feature_selector object is a disk_vector
     * and a collection of binary files. The disk_vector allows constant-time
     * access to look up a term_id and check whether it has been "selected". The
     * binary files are sorted by feature score for easy summary operations as
     * well as changing which top features are set to be selected.
     *
     * This base feature_selector class calculates and contains four
     * distributions which may be used to calculate different feature selection
     * scores, implemented as derived classes.
     */

    /**
     * Creates the state of this feature_selector if necessary; this logic is
     * outside the constructor since it requires pure virtual functions
     * implemented by deriving classes.
     * @param features_per_class
     */
    void init(uint64_t features_per_class);

    /// friend the factory function used to create feature_selectors, since
    /// they need to call the init
    friend std::unique_ptr<feature_selector>
        make_selector(const cpptoml::table& config,
                      std::shared_ptr<index::forward_index> idx);

    /**
     * Calculates the probabilities of terms and classes given the current
     * index.
     */
    void calc_probs();

    /**
     * Calculates the feature score for each (label, term) pair.
     */
    void score_all();

    /// Where the feature selection data is stored
    const std::string prefix_;

    /// The forward_index this feature selection is being performed on
    std::shared_ptr<index::forward_index> idx_;

    /// Whether or not a term_id is currently selected
    util::disk_vector<bool> selected_;

    /// P(t) in the entire collection, indexed by term_id
    std::vector<double> term_prob_;

    /// P(c) in the collection, indexed by label_id
    std::vector<double> class_prob_;

    /// P(c,t) indexed by [label_id][term_id]
    std::vector<std::vector<double>> co_occur_;
};
/**
 * Basic exception for feature selectors.
 */
class feature_selector_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#endif
