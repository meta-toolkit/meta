/**
 * @file parse_tree.cpp
 */

#include <fstream>
#include "tokenizers/tree/parse_tree.h"

namespace meta {
namespace tokenizers {

using std::stringstream;
using std::ifstream;
using std::endl;
using std::string;
using std::vector;

parse_tree::parse_tree(string tags):
    _children(vector<parse_tree>())
{
    _syntactic_category = root_category(tags);
    vector<string> trans(transitions(tags));
    for(auto & transition: trans)
        _children.push_back(parse_tree(transition));
}

vector<string> parse_tree::transitions(string tags) const
{
    // make sure there are actually transitions
    if(tags == "" || tags.substr(1, tags.size() - 1).find_first_of("(") == string::npos)
        return vector<string>();

    // get rid of first transition and its closing paren
    size_t index = 1;
    while(tags[index] != '(')
        ++index;
    tags = tags.substr(index, tags.size() - index - 1);

    // find groups of same-level parens; start on the first paren
    index = 0;
    int parenDepth = 0;
    string current = "";
    vector<string> transitions;
    while(index < tags.size())
    {
        current += tags[index];
        if(tags[index] == ')')
            --parenDepth;
        else if(tags[index] == '(')
            ++parenDepth;

        if(parenDepth == 0)
        {
            transitions.push_back(current);
            current = "";
        }
        ++index;
    }

    return transitions;
}

string parse_tree::root_category(string tags) const
{
    size_t index = 1;
    string POS = "";
    while(tags[index] != ')' && tags[index] != '(')
        POS += tags[index++];
    return POS;
}

string parse_tree::get_category() const
{
    return _syntactic_category;
}

vector<parse_tree> parse_tree::children() const
{
    return _children;
}

size_t parse_tree::num_children() const
{
    return _children.size();
}

string parse_tree::get_string() const
{
    string ret = "(" + _syntactic_category;
    for(auto & child: _children)
        ret += child.get_string();
    ret += ")";
    return ret;
}

string parse_tree::skeleton() const
{
    string ret = "(";
    for(auto & child: _children)
        ret += child.skeleton();
    ret += ")";
    return ret;
}

string parse_tree::pretty_print(const parse_tree & tree)
{
    stringstream output;
    pretty_print(tree, 0, output);
    return output.str();
}

void parse_tree::pretty_print(const parse_tree & tree, size_t depth, stringstream & output)
{
    string padding(depth, ' ');
    output << padding << "(" << endl << padding
           << "  " << tree._syntactic_category << endl;
    for(auto & child: tree._children)
        pretty_print(child, depth + 2, output);
    output << padding << ")" << endl;
}

string parse_tree::get_children_string() const
{
    string ret = "";
    for(auto & child: _children)
        ret += "(" + child._syntactic_category + ")";
    return ret;
}

vector<parse_tree> parse_tree::get_trees(const string & filename)
{
    vector<parse_tree> trees;
    ifstream treeFile(filename, ifstream::in);
    if(treeFile.is_open())
    {
        string line;
        while(treeFile.good())
        {
            std::getline(treeFile, line);
            if(line != "")
                trees.push_back(parse_tree(line));
        }        
        treeFile.close();
    }
    else
        throw parse_tree_exception("failed to open " + filename);

    return trees;
}

size_t parse_tree::height(const parse_tree & curr)
{
    size_t max = 0;
    for(auto & child: curr.children())
    {
        size_t h = height(child) + 1;
        if(h > max)
            max = h;
    }
    return max;
}

string parse_tree::get_skeleton_children() const
{
    string ret = "";
    for(size_t i = 0; i < _children.size(); ++i)
        ret += "()";
    return ret;
}

}
}
