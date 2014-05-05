/**
 * @file parse_tree.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_TREE_H_
#define META_PARSE_TREE_H_

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace meta
{
namespace corpus
{
class document;
}
namespace analyzers
{

/**
 * Represents a static parse tree that was generated from the Stanford
 *  parser.
 */
class parse_tree
{
  public:
    /**
     * Constructor.
     * Detects whether the parameter is a subtree or a leaf, and recursively
     *  builds subtrees.
     * @param tags The string representation of the tree to construct
     */
    parse_tree(const std::string& tags);

    /**
     * @return the toplevel part of speech for this parse_tree.
     */
    std::string get_category() const;

    /**
     * @return a vector of this parse_tree's immediate children.
     */
    std::vector<parse_tree> children() const;

    /**
     * @param curr The current subtree
     * @return the height of the current tree
     */
    static uint64_t height(const parse_tree& curr);

    /**
     * @return the number of immediate children for this parse_tree.
     */
    uint64_t num_children() const;

    /**
     * @return a string representation of the parse_tree.
     */
    std::string get_string() const;

    /**
     * @return a string representation of the parse_tree structure.
     */
    std::string skeleton() const;

    /**
     * @param tree The tree to print
     * @return a nice multiline string representation of the tree
     */
    static std::string pretty_print(const parse_tree& tree);

    /**
     * @return a string representation of the parse_tree's children.
     */
    std::string get_children_string() const;

    /**
     * @return a string representation of the parse_tree's children without
     * tags.
     */
    std::string get_skeleton_children() const;

    /**
     * @return the yield of this parse tree as a string
     */
    std::string yield() const;

    /**
     * @param filename Where to read the trees from
     * @return a vector of parse_trees generated from the given file
     * @param doc The document to read trees from
     */
    static std::vector<parse_tree> get_trees(const corpus::document& doc);

  private:
    /// the tag label on the root of this subtree
    std::string syntactic_category_;

    /// ordered collection of children of the current parse tree
    std::vector<parse_tree> children_;

    /**
     * @param doc The document to read trees from. This assumes the parse tree
     * objects are whitespace delimited.
     * @return a vector of parse_trees generated from the document's content
     */
    static std::vector<parse_tree> content_trees(const corpus::document& doc);

    /**
     * @param doc The document that represents the path to the trees
     * @return a vector of parse_trees generated from the document's file path
     */
    static std::vector<parse_tree> file_trees(const corpus::document& doc);

    /**
     * @return a vector of subtrees in string representation.
     */
    std::vector<std::string> transitions(std::string tags) const;

    /**
     * @return the root part of speech for a transition.
     */
    std::string root_category(const std::string& tags) const;

    /**
     * @param tree The parse_tree to print
     * @param depth The current depth (used for indentation)
     * @param output The stream to write to
     */
    static void pretty_print(const parse_tree& tree, uint64_t depth,
                             std::stringstream& output);

  public:
    /**
     * Basic exception for parse_tree interactions.
     */
    class parse_tree_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };
};
}
}

#endif
