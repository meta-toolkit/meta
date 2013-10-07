/**
 * @file document.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "meta.h"
#include "util/invertible_map.h"

namespace meta {
namespace corpus {

/**
 * Represents an indexable document.
 */
class document
{
    public:

        /**
         * Constructor.
         * @param path The path to the document
         */
        document(const std::string & path, doc_id d_id,
                 const class_label & label = class_label{""});

        /**
         * Increment the count of the specified transition.
         * @param termID - the token count to increment
         * @param amount - the amount to increment by
         */
        void increment(term_id termID, uint64_t amount);

        /**
         * Increment the count of the specified transition.
         * @param termID - the token count to increment
         * @param amount - the amount to increment by
         * @param docFreq - used for IDF
         */
        void increment(term_id termID, uint64_t amount,
                std::shared_ptr<std::unordered_map<term_id, uint64_t>> docFreq);

        /**
         * @return the path to this document (the argument to the constructor)
         */
        std::string path() const;

        /**
         * @return the classification category this document is in
         */
        class_label label() const;

        /**
         * @return the name of this document
         */
        std::string name() const;

        /**
         * @return the total of transitions recorded for this document.
         * This is not the number of unique transitions.
         */
        size_t length() const;

        /**
         * Get the number of occurrences for a particular transition.
         * @param termID - the termID of the term to look up
         */
        size_t frequency(term_id termID) const;

        /**
         * @return the map of frequencies for this document.
         */
        const std::unordered_map<term_id, uint64_t> & frequencies() const;
 
        /**
         * Removes featuress from a document.
         * @param docs The documents to remove features from
         * @param features A list of features that should remain in the document
         * @return the filtered document
         */
        static document filter_features(const document & doc,
                                        const std::vector<
                                            std::pair<term_id, double>
                                        > & features);

        /**
         * Removes features from each document.
         * @param docs The documents to remove features from
         * @param features A list of features that should be removed from the
         * document
         * @return the filtered documents
         */
        static std::vector<document> filter_features(
                const std::vector<document> & docs,
                const std::vector<std::pair<term_id, double>> & features);

        /**
         * Outputs class label integer for slda.
         * @param mapping Keeps track of class labels as integers.
         */
        std::string get_slda_label_data(
                util::invertible_map<class_label, int> & mapping) const;

        /**
         * Outputs term count data in slda format.
         */
        std::string get_slda_term_data() const;

        /**
         * Wrapper function for a document's cosine similarity measure.
         * @param a
         * @param b
         * @return the Jaccard similarity between the two parameters
         */
        static double jaccard_similarity(const document & a,
                                         const document & b);

        /**
         * Wrapper function for a document's cosine similarity measure.
         * @param a
         * @param b
         * @return the cosine similarity between the two parameters
         */
        static double cosine_similarity(const document & a, const document & b);

        /**
         * Sets the content of the document to be the parameter
         * @param content
         * @note saving the document's content is only used by some corpora
         * formats; not all documents are guaranteed to have content stored in
         * the object itself
         */
        void set_content(const std::string & content);

        /**
         * @return the contents of this document
         */
        const std::string & content() const;

        /**
         * @return the doc_id for this document
         */
        doc_id id() const;

        /**
         * @return whether this document contains its content internally
         */
        bool contains_content() const;

    private:

        /** where this document is on disk */
        std::string _path;

        /** the document id for this document */
        doc_id _d_id;

        /** which category this document would be classified into */
        class_label _label;

        /** the short name for this document (not the full path) */
        std::string _name;

        /** the number of (non-unique) tokens in this document */
        size_t _length;

        /** counts of how many times each token appears */
        std::unordered_map<term_id, uint64_t> _frequencies;

        /** what the document contains */
        std::string _content;

        /** indicates whether this document has the original content stored in
         * it */
        bool _contains_content;
};

}
}

#endif
