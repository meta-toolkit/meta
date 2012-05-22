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
         *
         */
        ParseTree(string text);

        /**
         *
         */
        static vector<ParseTree> getTrees(string filename);

    private:

};

#endif
