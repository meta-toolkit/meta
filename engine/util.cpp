#include "util.h"

/**
 * @param filename
 * @return - a vector of ParseTrees
 */
vector<ParseTree> util::getTrees(string filename)
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
