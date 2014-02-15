/**
 * @file parse_tree.cpp
 */

#include <fstream>

#include "analyzers/tree/parse_tree.h"

namespace meta {
namespace analyzers {

parse_tree::parse_tree(const std::string & tags)
{
    _syntactic_category = root_category(tags);
    std::vector<std::string> trans{transitions(tags)};
    for(auto & transition: trans)
        _children.push_back(parse_tree(transition));
}

std::vector<std::string> parse_tree::transitions(std::string tags) const
{
    // make sure there are actually transitions
    if(tags == "" ||
       tags.substr(1, tags.size() - 1).find_first_of("(") == std::string::npos)
        return std::vector<std::string>{};

    // get rid of first transition and its closing paren
    uint64_t index = 1;
    while(tags[index] != '(')
        ++index;
    tags = tags.substr(index, tags.size() - index - 1);

    // find groups of same-level parens; start on the first paren
    index = 0;
    int paren_depth = 0;
    std::string current{""};
    std::vector<std::string> transitions;
    while(index < tags.size())
    {
        current += tags[index];
        if(tags[index] == ')')
            --paren_depth;
        else if(tags[index] == '(')
            ++paren_depth;

        if(paren_depth == 0)
        {
            transitions.push_back(current);
            current = "";
        }
        ++index;
    }

    return transitions;
}

std::string parse_tree::root_category(const std::string & tags) const
{
    uint64_t index = 1;
    std::string cat{""};
    while(tags[index] != ')' && tags[index] != '(')
        cat += tags[index++];
    return cat;
}

std::string parse_tree::get_category() const
{
    return _syntactic_category;
}

std::vector<parse_tree> parse_tree::children() const
{
    return _children;
}

uint64_t parse_tree::num_children() const
{
    return _children.size();
}

std::string parse_tree::get_string() const
{
    std::string ret{"(" + _syntactic_category};
    for(auto & child: _children)
        ret += child.get_string();
    ret += ")";
    return ret;
}

std::string parse_tree::skeleton() const
{
    std::string ret{"("};
    for(auto & child: _children)
        ret += child.skeleton();
    ret += ")";
    return ret;
}

std::string parse_tree::pretty_print(const parse_tree & tree)
{
    std::stringstream output;
    pretty_print(tree, 0, output);
    return output.str();
}

void parse_tree::pretty_print(const parse_tree & tree, uint64_t depth,
        std::stringstream & output)
{
    std::string padding(depth, ' ');
    output << padding << "(" << std::endl << padding
           << "  " << tree._syntactic_category << std::endl;
    for(auto & child: tree._children)
        pretty_print(child, depth + 2, output);
    output << padding << ")" << std::endl;
}

std::string parse_tree::get_children_string() const
{
    std::string ret{""};
    for(auto & child: _children)
        ret += "(" + child._syntactic_category + ")";
    return ret;
}

std::vector<parse_tree> parse_tree::get_trees(const std::string & filename)
{
    std::vector<parse_tree> trees;
    std::ifstream treefile{filename, std::ifstream::in};
    if(treefile.is_open())
    {
        std::string line;
        while(treefile >> line)
            trees.push_back(parse_tree{line});
        treefile.close();
    }
    else
        throw parse_tree_exception("failed to open " + filename);

    return trees;
}

uint64_t parse_tree::height(const parse_tree & curr)
{
    uint64_t max = 0;
    for(auto & child: curr.children())
    {
        uint64_t h = height(child) + 1;
        if(h > max)
            max = h;
    }
    return max;
}

std::string parse_tree::get_skeleton_children() const
{
    std::string ret{""};
    for(uint64_t i = 0; i < _children.size(); ++i)
        ret += "()";
    return ret;
}

}
}
