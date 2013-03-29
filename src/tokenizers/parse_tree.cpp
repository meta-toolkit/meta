/**
 * @file parse_tree.cpp
 */

#include <fstream>
#include "tokenizers/parse_tree.h"

namespace meta {
namespace tokenizers {

using std::stringstream;
using std::ifstream;
using std::endl;
using std::string;
using std::vector;

ParseTree::ParseTree(string tags): children(vector<ParseTree>())
{
    partOfSpeech = getRootPOS(tags);
    vector<string> transitions(getTransitions(tags));
    for(auto & transition: transitions)
        children.push_back(ParseTree(transition));
}

vector<string> ParseTree::getTransitions(string tags) const
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

string ParseTree::getRootPOS(string tags) const
{
    size_t index = 1;
    string POS = "";
    while(tags[index] != ')' && tags[index] != '(')
        POS += tags[index++];
    return POS;
}

string ParseTree::getPOS() const
{
    return partOfSpeech;
}

vector<ParseTree> ParseTree::getChildren() const
{
    return children;
}

size_t ParseTree::numChildren() const
{
    return children.size();
}

string ParseTree::getString() const
{
    string ret = "(" + partOfSpeech;
    for(auto & child: children)
        ret += child.getString();
    ret += ")";
    return ret;
}

string ParseTree::getSkeleton() const
{
    string ret = "(";
    for(auto & child: children)
        ret += child.getSkeleton();
    ret += ")";
    return ret;
}

string ParseTree::prettyPrint(const ParseTree & tree)
{
    stringstream output;
    prettyPrint(tree, 0, output);
    return output.str();
}

void ParseTree::prettyPrint(const ParseTree & tree, size_t depth, stringstream & output)
{
    string padding(depth, ' ');
    output << padding << "(" << endl << padding
           << "  " << tree.partOfSpeech << endl;
    for(auto & child: tree.children)
        prettyPrint(child, depth + 2, output);
    output << padding << ")" << endl;
}

string ParseTree::getChildrenString() const
{
    string ret = "";
    for(auto & child: children)
        ret += "(" + child.partOfSpeech + ")";
    return ret;
}

vector<ParseTree> ParseTree::getTrees(const string & filename)
{
    vector<ParseTree> trees;
    ifstream treeFile(filename, ifstream::in);
    if(treeFile.is_open())
    {
        string line;
        while(treeFile.good())
        {
            std::getline(treeFile, line);
            if(line != "")
                trees.push_back(ParseTree(line));
        }        
        treeFile.close();
    }
    else
        throw ParseTreeException("failed to open " + filename);

    return trees;
}

size_t ParseTree::height(const ParseTree & curr)
{
    size_t max = 0;
    for(auto & child: curr.getChildren())
    {
        size_t h = height(child) + 1;
        if(h > max)
            max = h;
    }
    return max;
}

string ParseTree::getSkeletonChildren() const
{
    string ret = "";
    for(size_t i = 0; i < children.size(); ++i)
        ret += "()";
    return ret;
}

}
}
