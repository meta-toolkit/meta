/**
 * @file select.h
 */

#ifndef _SELECT_H_
#define _SELECT_H_

#include <vector>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "index/document.h"

namespace meta {
namespace classify {
    
/**
 * Contains various feature selection methods.
 */
namespace feature_select {

    /**
     * Separates documents into class buckets.
     * @param docs The Documents to filter through
     * @return the bucketed Documents
     */
    std::unordered_map<std::string, std::vector<index::Document>> partition_classes(
            const std::vector<index::Document> & docs);
    /**
     * @param docs
     * @return all classes used in the corpus represented by the vector of
     * Documents
     */
    std::unordered_set<std::string> get_class_space(const std::vector<index::Document> & docs);

    /**
     * @param docs
     * @return all terms used in the corpus represented by the vector of
     * Documents
     */
    std::unordered_set<index::TermID> get_term_space(const std::vector<index::Document> & docs);

    /**
     * Calculates percentage of documents in class c in which term t occurs.
     * @param term
     * @param label
     * @param classes
     * @return P(t, c), or 1 - P(t', c)
     */
    double term_given_class(index::TermID term, const std::string & label,
            std::unordered_map<std::string, std::vector<index::Document>> classes);

    /**
     * Calculates percentage of documents not in class c in which term t does
     * not occur.
     * @param term
     * @param label
     * @param classes
     * @return P(t', c'), or 1 - P(t, c')
     */
    double not_term_given_not_class(index::TermID term, const std::string & label,
            std::unordered_map<std::string, std::vector<index::Document>> classes);

}

}
}

#endif
