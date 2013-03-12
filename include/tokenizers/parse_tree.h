/**
 * @file parse_tree.h
 */

#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_

#include <sstream>
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
         * @param curr
         * @return the height of the current tree
         */
        static size_t height(const ParseTree & curr);

        /**
         * @return the number of immediate children for this ParseTree.
         */
        size_t numChildren() const;

        /**
         * @return a string representation of the ParseTree.
         */
        std::string getString() const;

        /**
         * @return a string representation of the ParseTree structure.
         */
        std::string getSkeleton() const;

        /**
         * @param tree - the tree to print
         * @return a nice multiline string representation of the tree
         */
        static std::string prettyPrint(const ParseTree & tree);

        /**
         * @return a string representation of the ParseTree's children.
         */
        std::string getChildrenString() const;

        /**
         * @return a string representation of the ParseTree's children without tags.
         */
        std::string getSkeletonChildren() const;

        /**
         * @param filename - where to read the trees from
         * @return a vector of ParseTrees generated from the given file
         */
        static std::vector<ParseTree> getTrees(const std::string & filename);

    private:

        /** the tag label on the root of this subtree */
        std::string partOfSpeech;

        /** ordered collection of children of the current parse tree */
        std::vector<ParseTree> children;

        /**
         * @return a vector of subtrees in string representation.
         */
        std::vector<std::string> getTransitions(std::string tags) const;

        /**
         * @return the root part of speech for a transition.
         */
        std::string getRootPOS(std::string tags) const;

        /**
         * @param tree
         * @param depth
         * @param output
         */
        static void prettyPrint(const ParseTree & tree, size_t depth,
                std::stringstream & output);
};

/**
 * Basic exception for ParseTree interactions.
 */
class ParseTreeException: public std::exception
{
    public:
        
        ParseTreeException(const std::string & error):
            _error(error) { /* nothing */ }

        const char* what () const throw ()
        {
            return _error.c_str();
        }
   
    private:
   
        std::string _error;
};

#endif
