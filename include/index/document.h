/**
 * @file document.h
 */

#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "structs.h"
#include "util/invertible_map.h"

namespace meta {
namespace index {

/**
 * Represents an indexed document.
 */
class Document
{
    public:

        /**
         * Constructor.
         * @param path - the path to the document
         */
        Document(const std::string & path);

        /**
         * Increment the count of the specified transition.
         * @param termID - the token count to increment
         * @param amount - the amount to increment by
         */
        void increment(TermID termID, unsigned int amount);

        /**
         * Increment the count of the specified transition.
         * @param termID - the token count to increment
         * @param amount - the amount to increment by
         * @param docFreq - used for IDF
         */
        void increment(TermID termID, unsigned int amount,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

        /**
         * @return the path to this document (the argument to the constructor)
         */
        std::string getPath() const;

        /**
         * @return the classification category this document is in
         */
        std::string getCategory() const;

        /**
         * @return the containing directory of a file given its full path
         */
        static std::string getCategory(const std::string & path);

        /**
         * @return the name of this document
         */
        std::string getName() const;

        /**
         * @return the total of transitions recorded for this Document.
         * This is not the number of unique transitions.
         */
        size_t getLength() const;

        /**
         * Get the number of occurrences for a particular transition.
         * @param termID - the termID of the term to look up
         */
        size_t getFrequency(TermID termID) const;

        /**
         * @return the map of frequencies for this document.
         */
        const std::unordered_map<TermID, unsigned int> & getFrequencies() const;
 
        /**
         * Prints tokenizer output in liblinear input format.
         * @param mapping Keeps track of class labels as integers.
         * @param usingSLDA Indicates which learner we're passing the data to.
         * @return
         */
        std::string getLearningData(util::InvertibleMap<std::string, int> & mapping, bool usingSLDA) const;

        std::string getFilteredLearningData(util::InvertibleMap<std::string, int> & mapping, 
                const std::unordered_set<TermID> & features) const;

        /**
         * Wrapper function for a Document's cosine similarity measure.
         * @param a
         * @param b
         * @return the Jaccard similarity between the two parameters
         */
        static double jaccard_similarity(const Document & a, const Document & b);

        /**
         * Wrapper function for a Document's cosine similarity measure.
         * @param a
         * @param b
         * @return the cosine similarity between the two parameters
         */
        static double cosine_similarity(const Document & a, const Document & b);

        /**
         * Returns a vector of all documents in a given dataset.
         * @param filename - the file containing the list of files in a corpus
         * @param prefix - the prefix of the path to a corpus
         * @return a vector of Documents created from the filenames
         */
        static std::vector<Document> loadDocs(const std::string & filename,
                const std::string & prefix);

    private:

        /** where this document is on disk */
        std::string _path;

        /** which category this document would be classified into */
        std::string _category;

        /** the short name for this document (not the full path) */
        std::string _name;

        /** the number of (non-unique) tokens in this document */
        size_t _length;

        /** counts of how many times each token appears */
        std::unordered_map<TermID, unsigned int> _frequencies;

        /**
         * @return the name of a document given its full path
         */
        static std::string getName(const std::string & path);

        /**
         * @param mapping - an InvertibleMap of category <-> category id
         * @param category - the category to find an id for
         * @return a unique numerical id for the category, creating a new entry if necessary
         */
        static int getMapping(util::InvertibleMap<std::string, int> & mapping, const std::string & category);
};

}
}

#endif
