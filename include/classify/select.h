/**
 * @file select.h
 */

#ifndef _SELECT_H_
#define _SELECT_H_

#include <vector>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include "index/document.h"
#include "meta.h"

namespace meta {
namespace classify {
    
/**
 * Shared feature selection utilites that inheriting classes use.
 */
class feature_select
{
    public:

        /**
         * Constructor; initializes class and term probabilities.
         */
        feature_select(const std::vector<index::Document> & docs);

        /**
         * Default constructor in case a feature selection method does want
         * to use functionality in the base class.
         */
        feature_select() = default;

        /**
         * Performs feature selection on a collection of Documents.
         * @param docs A vector of tokenized Documents containing all features
         * @return a vector of TermIDs, sorting by their feature selection
         * rating
         */
        virtual std::vector<std::pair<TermID, double>> select() = 0;

        /**
         * Performs feature selection on a collection of Documents, returning
         * each class's features sorted by usefulness.
         */
        virtual std::unordered_map<ClassLabel, std::vector<std::pair<TermID, double>>>
            select_by_class() = 0;

    protected:

        /**
         * Probability of term occuring in class
         * \f$ P(t, c) = \frac{c(t, c)}{T} \f$
         * @param term
         * @param label
         * @return P(t, c)
         */
        double term_and_class(TermID term, const ClassLabel & label) const;

        /**
         * Probability of not seeing a term and a class:
         * \f$ P(t', c) = P(c) - P(t, c) \f$
         * @param term
         * @param label
         * @return P(t', c)
         */
        double not_term_and_class(TermID term, const ClassLabel & label) const;

        /**
         * Probability of term not occuring in a class:
         * \f$ P(t, c') = P(t) - P(t, c) \f$
         * @param term
         * @param label
         * @return P(t, c')
         */
        double term_and_not_class(TermID term, const ClassLabel & label) const;

        /**
         * Probability not in class c in which term t does not occur:
         * \f$ P(t', c') = 1 - P(t, c) - P(t', c) - P(t, c') \f$
         * @param term
         * @param label
         * @return P(t', c')
         */
        double not_term_and_not_class(TermID term, const ClassLabel & label) const;

        /** sorts terms by term weight */
        std::vector<std::pair<TermID, double>>
            sort_terms(std::unordered_map<TermID, double> & weights) const;

        /** all unique terms */
        std::unordered_set<TermID> _term_space;

        /** all unique classes */
        std::unordered_set<ClassLabel> _class_space;

        /** number of total (not unique) terms */
        size_t _num_terms;

        /** probability of a term in the corpus */
        std::unordered_map<TermID, double> _pterm;

        /** probability of a class in the corpus */
        std::unordered_map<ClassLabel, double> _pclass;

    private:

        /**
         * Probability of a word and class co-occuring.
         */
        std::unordered_map<ClassLabel, std::unordered_map<TermID, double>> _pseen;

        /**
         * Calculates probabilities for terms and classes co-occuring.
         */
        void set_pseen(const std::vector<index::Document> & docs);
        
        /**
         * Calculates the term space of the corpus.
         */
        void set_term_space(const std::vector<index::Document> & docs);

        /**
         * Calculates the class space of the corpus.
         */
        void set_class_space(const std::vector<index::Document> & docs);
};

}
}

#endif
