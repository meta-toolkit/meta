/**
 * @file confusion_matrix.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CONFUSION_MATRIX_H_
#define META_CONFUSION_MATRIX_H_

#include <set>
#include "index/forward_index.h"
#include "meta.h"

namespace meta
{
namespace classify
{

/**
 * Allows interpretation of classification errors.
 */
class confusion_matrix
{
  public:
    /**
     * Creates an empty confusion matrix.
     */
    confusion_matrix();

    /**
     * @param predicted The predicted class label
     * @param actual The actual class label
     * @param times The number of times this prediction was made
     */
    void add(const class_label& predicted, const class_label& actual,
             size_t times = 1);

    /**
     * Prints this matrix's statistics to out.
     *
     * @param out The stream to write to (defaults to `std::cout`)
     */
    void print_stats(std::ostream& out = std::cout) const;

    /**
     * Prints this matrix to out.
     *
     * @param out The stream to write to (defaults to `std::cout`)
     */
    void print(std::ostream& out = std::cout) const;

    /**
     * Prints (predicted, actual) pairs for all judgements
     *
     * @param out The stream to write to (defaults to `std::cout`)
     */
    void print_result_pairs(std::ostream& out = std::cout) const;

    /**
     * Implements a hash function for a pair of strings.
     * @param str_pair The pair of strings
     * @return the hash
     */
    static size_t
        string_pair_hash(const std::pair<std::string, std::string>& strPair);

    // note: the following *cannot* be converted to a using declaration
    // without causing in internal compiler error (segmentation fault) in
    // GCC 4.8.2
    /** typedef for predicted class assignments to counts. */
    typedef std::unordered_map<std::pair<class_label, class_label>, size_t,
                               decltype(&confusion_matrix::string_pair_hash)>
        prediction_counts;

    /**
     * @return all the predictions from this confusion_matrix.
     */
    const prediction_counts& predictions() const;

    /**
     * @return the accuracy for this confusion matrix
     */
    double accuracy() const;

    /**
     * operator+ for confusion matrices. All counts are agglomerated for all
     * predictions.
     * @param other
     * @return a confusion_matrix containing all predictions of the parameters
     */
    confusion_matrix operator+(const confusion_matrix& other) const;

    /**
     * operator+= for confusion matrices. All counts are agglomerated for all
     * predictions.
     * @param other
     * @return a confusion_matrix containing all predictions of the parameters
     */
    confusion_matrix& operator+=(const confusion_matrix& other);

    /**
     * @param a The first matrix to compare
     * @param b The second matrix to compare
     * @return whether results between two confusion matrices are
     * statistically significant according to McNemar's test with Yates'
     * correction for continuity (alpha = .05)
     */
    static bool mcnemar_significant(const confusion_matrix& a,
                                    const confusion_matrix& b);

  private:
    /**
     * Prints precision, recall, and F1 for each class and as a whole.
     * @param out The stream to print to
     * @param label The current class label to get statistics for
     * @param prec The precision for this class
     * @param rec The recall for this class
     * @param f1 The F1 score for this class
     */
    void print_class_stats(std::ostream& out, const class_label& label,
                           double& prec, double& rec, double& f1,
                           size_t width) const;

    /** maps predicted class to actual class frequencies */
    prediction_counts predictions_;

    /**
     * Keeps track of the number of classes. We use a std::set here so the
     * class labels are sorted alphabetically.
     */
    std::set<class_label> classes_;

    /** how many times each class was predicted */
    std::unordered_map<class_label, size_t> counts_;

    /** total number of classification attempts */
    size_t total_;
};
}
}

#endif
