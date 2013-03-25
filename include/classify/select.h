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

namespace classify { namespace feature_select {

    /**
     * @param docs
     * @return
     */
    std::unordered_map<std::string, std::vector<Document>> partition_classes(
            const std::vector<Document> & docs);

    /**
     * @param docs
     * @return
     */
    std::unordered_set<TermID> get_term_space(const std::vector<Document> & docs);

    /**
     * Calculates percentage of documents in class c in which term t occurs.
     * @param term
     * @param label
     * @param classes
     * @return P(t, c)
     */
    double term_given_class(TermID term, const std::string & label,
            std::unordered_map<std::string, std::vector<Document>> classes);

} }

#endif
