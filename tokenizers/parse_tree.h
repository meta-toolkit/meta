/**
 * @file parse_tree.h
 */

#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_

#include <string>
#include <vector>

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
        ParseTree(std::string tags);

        /**
         * @return the toplevel part of speech for this ParseTree.
         */
        std::string getPOS() const;

        /**
         * @return a vector of this ParseTree's immediate children.
         */
        std::vector<ParseTree> getChildren() const;

        /**
         * @return the number of immediate children for this ParseTree.
         */
        size_t numChildren() const;

        /**
         * @return a string representation of the ParseTree.
         */
        std::string getString() const;

        /**
         * @return a string representation of the ParseTree's children.
         */
        std::string getChildrenString() const;

        /**
         * @param filename - where to read the trees from
         * @return a vector of ParseTrees generated from the given file
         */
        static std::vector<ParseTree> getTrees(const std::string & filename);

    private:

        std::string partOfSpeech;
        std::vector<ParseTree> children;

        /**
         * @return a vector of subtrees in string representation.
         */
        std::vector<std::string> getTransitions(std::string tags) const;

        /**
         * @return the root part of speech for a transition.
         */
        std::string getRootPOS(std::string tags) const;
};

#endif
