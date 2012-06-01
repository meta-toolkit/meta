/**
 * @file parse_tree.h
 */

#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::ifstream;
using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::vector;

/**
 * Represents a static parse tree that was generated from the Stanford
 *  Parser.
 */
class ParseTree
{
    public:

        /**
         * Constructor.
         * Detects whether the parameter is a subtree or a leaf, and recursively
         *  builds subtrees.
         */
        ParseTree(string tags);

        /**
         * @return the toplevel part of speech for this ParseTree.
         */
        string getPOS() const;

        /**
         * @return a vector of this ParseTree's immediate children.
         */
        vector<ParseTree> getChildren() const;

        /**
         * @return the number of immediate children for this ParseTree.
         */
        size_t numChildren() const;

        /**
         * @return a string representation of the ParseTree.
         */
        string getString() const;

        /**
         * @return a string representation of the ParseTree's children.
         */
        string getChildrenString() const;

        /**
         * @param filename - where to read the trees from
         * @return a vector of ParseTrees generated from the given file
         */
        static vector<ParseTree> getTrees(const string & filename);

    private:

        string partOfSpeech;
        vector<ParseTree> children;

        /**
         * @return a vector of subtrees in string representation.
         */
        vector<string> getTransitions(string tags) const;

        /**
         * @return the root part of speech for a transition.
         */
        string getRootPOS(string tags) const;
};

#endif
