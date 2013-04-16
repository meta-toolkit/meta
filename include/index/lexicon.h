/**
 * @file lexicon.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _LEXICON_H_
#define _LEXICON_H_

#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <string>
#include <utility>
#include <sstream>

#include "meta.h"
#include "structs.h"
#include "util/invertible_map.h"

namespace meta {
namespace index {

/**
 * Represents the dictionary or lexicon of an inverted index.
 */
class Lexicon
{
    public:

        /**
         * Constructor to read an existing lexicon from disk.
         */
        Lexicon(const std::string & lexiconFile);

        /**
         * @return whether this Lexicon is empty
         */
        bool isEmpty() const;

        /**
         * @return all lexicon information about a specific term.
         */
        TermData getTermInfo(term_id termID) const;

        /**
         * Writes the lexicon to disk.
         * @param docLengthsFilename
         * @param termMapFilename
         * @param docMapFilename
         */
        void save(const std::string & docLengthsFilename, const std::string & termMapFilename, const std::string & docMapFilename) const;

        /**
         * Adds a new term to the lexicon.
         */
        void addTerm(term_id term, TermData termData);

        /**
         * @param docID - the id of the document to get the length of
         * @return the length of the parameter document
         */
        unsigned int getDocLength(doc_id docID) const;

        /**
         * @return the number of documents in this collection
         */
        unsigned int getNumDocs() const;

        /**
         * @return the average document length in the collection
         */
        double getAvgDocLength() const;

        /**
         * @return the string term associated with the termID
         */
        std::string getTerm(term_id termID) const;

        /**
         * @return the term_id associated with the given term
         */
        term_id getterm_id(std::string term) const;
        
        /**
         * @return whether this lexicon has information on the specified termID
         */
        bool containsterm_id(term_id termID) const;

        /**
         * @return the string document name associated with the termID
         */
        std::string getDoc(doc_id docID) const;
        
        /**
         * @return the doc_id of the given document name
         */
        doc_id getdoc_id(std::string docName) const;

        /**
         * Reads document lengths from disk into memory.
         * We don't use InvertibleMap::readMap because many docLengths
         *  can be duplicated.
         */
        void setDocLengths(const std::string & filename);

        /**
         * @return the term_id mapping for this lexicon
         */
        const util::InvertibleMap<term_id, std::string> & getterm_idMapping() const;

    private:

        /** the name of the lexicon file */
        std::string _lexiconFilename;

        /** saves the average document length in this collection */
        double _avgDL;

        /** maps term_id (tokens) -> TermData (where to find in the postingsFile) */
        std::unordered_map<term_id, TermData> _entries;

        /** lengths for all documents in the index, used in ranking functions */
        std::unordered_map<doc_id, unsigned int> _docLengths;

        /** maps term_ids to the strings they represent */
        util::InvertibleMap<term_id, std::string> _termMap;

        /** maps doc_ids to the document paths they represent */
        util::InvertibleMap<doc_id, std::string> _docMap;

        /**
         * Reads a lexicon from disk if it exists.
         * This function is called from the lexicon constructor as
         *  well as the InvertedIndex constructor.
         */
        void readLexicon();

        /**
         * Calculates the average document length of the collection
         *  and stores it in the member variable.
         */
        void setAvgDocLength();
};

}
}

#endif
