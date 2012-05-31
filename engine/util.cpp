#include "engine.h"

/**
 * @param filename
 * @return - a vector of ParseTrees
 */
vector<ParseTree> engine::util::getTrees(string filename)
{
    cout << " Getting parse trees for " << filename << endl;
    vector<ParseTree> trees;
    ifstream treeFile(filename, ifstream::in);
    if(treeFile.is_open())
    {
        string line;
        while(treeFile.good())
        {
            std::getline(treeFile, line);
            trees.push_back(ParseTree(line));
        }        
        treeFile.close();
    }
    else
    {
        cerr << "[engine::util::getTrees]: Failed to open "
             << filename << endl;
    }

    return trees;
}

/**
 * Adds tokens from the given tree to the supplied document.
 * @param tree - the tree to tokenize
 * @param document - the document to add counts to
 */
void engine::util::tokenize(const ParseTree & tree, Document & document)
{
    return;
}
