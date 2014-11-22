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
#include <fstream>
#include <vector>
#include <memory>

#include "util/disk_vector.h"
#include "index/forward_index.h"
#include "io/binary.h"

namespace meta
{
namespace features
{
/**
 * The base class that shows the feature selection interface for MeTA, allowing
 * dimensionality reduction for documents as well as descriptions of classes by
 * their useful features.
 */
class feature_selector
{
  public:
    /**
     * @param filename
     * @param idx
     */
    feature_selector(const std::string& prefix,
                     std::shared_ptr<index::forward_index> idx);

    /**
     * Default destructor.
     */
    virtual ~feature_selector() = default;

    /**
     * Prints a summary of which features are representative for each class.
     */
    void print_summary() const;

  protected:
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

    /// Where the feature selection data is stored
    const std::string prefix_;

    /// The forward_index this feature selection is being performed on
    std::shared_ptr<index::forward_index> idx_;

    /// Whether or not a term_id is currently selected
    util::disk_vector<bool> selected_;

    /// Binary files containing features sorted by scores, indexed by label_id
    std::vector<std::ifstream> sorted_features_;

    /** The following three classes are only used during initial creation */

    /// P(t) in the entire collection, indexed by term_id
    std::vector<double> term_prob_;

    /// P(c) in the collection, indexed by label_id
    std::vector<double> class_prob_;

    /// P(c,t) indexed by label_id and term_id
    std::vector<std::vector<double>> co_occur_;
};
}
}

#endif
