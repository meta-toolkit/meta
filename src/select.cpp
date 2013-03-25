/**
 * @file select.h
 */

#include "classify/select.h"

namespace classify { namespace feature_select {

    typedef std::string ClassID;

    /**
     * Percentage of documents belonging to class c in which term occurs.
     */
    unordered_map<pair<TermID, ClassID>, double>
        term_in_class(const vector<Document> & docs);

    /**
     * Percentage of documents not belonging to class c in which term does not
     * occur.
     */
    unordered_map<pair<TermID, ClassID>, double>
        (const vector<Document> & docs);

    /**
     * Percentage of documents belonging to class c in which term does not
     * occur.
     */
    unordered_map<pair<TermID, ClassID>, double>
        not_term_in_class(const vector<Document> & docs);

    /**
     * Percentage of documents not belonging to class c in which term occurs.
     */
    unordered_map<pair<TermID, ClassID>, double>
        term_not_in_class(const vector<Document> & docs);

} }
