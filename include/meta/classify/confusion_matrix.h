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

#include <iostream>
#include <set>
#include <unordered_map>

#include "meta/config.h"
#include "meta/meta.h"

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
    void add(const predicted_label& predicted, const class_label& actual,
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
    string_pair_hash(const std::pair<std::string, std::string>& str_pair);

    // note: the following *cannot* be converted to a using declaration
    // without causing in internal compiler error (segmentation fault) in
    // GCC 4.8.2
    /** typedef for predicted class assignments to counts. */
    typedef std::unordered_map<std::pair<predicted_label, class_label>, size_t,
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
     * @return the overall F1 score
     */
    double f1_score() const;

    /**
     * @param lbl
     * @return the F1 score for the lbl class
     */
    double f1_score(const class_label& lbl) const;

    /**
     * @return the overall precision
     */
    double precision() const;

    /**
     * @return the precision for the lbl class
     */
    double precision(const class_label& lbl) const;

    /**
     * @return the overall recall
     */
    double recall() const;

    /**
     * @return the recall for the lbl class
     */
    double recall(const class_label& lbl) const;

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
    /// Maps predicted class to actual class frequencies
    prediction_counts predictions_;

    /**
     * Keeps track of the number of classes. We use a std::set here so the
     * class labels are sorted alphabetically.
     */
    std::set<class_label> classes_;

    /// How many times each class appears in the dataset
    std::unordered_map<class_label, size_t> counts_;

    /// Total number of classification attempts
    size_t total_;
};
}
}

#endif
