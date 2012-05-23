#include "parse_tree.h"

ParseTree::ParseTree(string tags): children(vector<ParseTree>())
{
    vector<string> transitions = getTransitions(tags);
    vector<string>::iterator iter;
    for(iter = transitions.begin(); iter != transitions.end(); ++iter)
        children.push_back(ParseTree(*iter));

    partOfSpeech = getRootPOS(tags);
}

vector<string> ParseTree::getTransitions(string tags) const
{
    // make sure there are actually transitions
    if(tags.substr(1, tags.size() - 1).find_first_of("(") == string::npos)
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
    vector<ParseTree>::const_iterator child;
    for(child = children.begin(); child != children.end(); ++child)
        ret += child->getString();
    ret += ")";
    return ret;
}

string ParseTree::getChildrenString() const
{
    string ret = "";
    vector<ParseTree>::const_iterator child;
    for(child = children.begin(); child != children.end(); ++child)
        ret += "(" + child->partOfSpeech + ")";
    return ret;
}
