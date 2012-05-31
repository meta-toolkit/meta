/**
 * @file engine.h
 */

#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <vector>
#include <string>
#include <fstream>
#include "parse_tree.h"
#include "document.h"

using std::ifstream;
using std::vector;
using std::string;

/**
 * Contains all components of the C++ portion of the project.
 */
namespace engine
{
    /**
     *
     */
    namespace index
    {
        void createIndex(string indexPath);
    }

    /**
     *
     */
    namespace search
    {
        /**
         *
         */
        void readIndex(string indexPath);

        /**
         * Scores a document given a query.
         * @param document - the doc to score
         * @param query - the query to score against
         * @return the real score value 
         */
        double scoreDocument(const Document & document, const Document & query);
    }

    /**
     * Contains functions used by both index and search.
     */
    namespace util
    {
        /**
         * @param filename
         * @return - a vector of ParseTrees
         */
        vector<ParseTree> getTrees(string filename);

        /**
         * Adds tokens from the given tree to the supplied document.
         * @param tree - the tree to tokenize
         * @param document - the document to add counts to
         */
        void tokenize(const ParseTree & tree, Document & document);
    }
}

#endif
