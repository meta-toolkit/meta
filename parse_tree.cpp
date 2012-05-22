#include "parse_tree.h"

ParseTree::ParseTree(string POS)
{
    partOfSpeech = POS;
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

void ParseTree::addChild(string POS)
{
    children.push_back(POS);
}
