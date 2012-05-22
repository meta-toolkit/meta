/**
 * @file parse_tree.h
 */

#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_

#include <string>
#include <vector>

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
         * Sets the part of speech to the parameter.
         */
        ParseTree(string POS);

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

    private:

        string partOfSpeech;
        vector<ParseTree> children;

        /**
         * Adds a subtree to the current ParseTree.
         * @param POS - the part of speech to give the subtree
         */
        void addChild(string POS);
};

#endif
