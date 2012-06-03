#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <map>

#include "tokenizer.h"
#include "pos_tree_tokenizer.h"
#include "document.h"
#include "parse_tree.h"
#include "ram_index.h"
#include "search.h"

using std::pair;
using std::make_pair;
using std::multimap;
using std::vector;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
    string prefix = "/home/sean/projects/senior-thesis-data/input/";

    vector<string> indexFiles;
    indexFiles.push_back(prefix + "doc1.txt.tree");
    indexFiles.push_back(prefix + "doc2.txt.tree");
    indexFiles.push_back(prefix + "doc3.txt.tree");
    indexFiles.push_back(prefix + "doc4.txt.tree");

    Tokenizer* tokenizer = new POSTreeTokenizer();
    RAMIndex index(indexFiles, tokenizer);
    delete tokenizer;

    Document query("author", "nationality");
    multimap<double, string> results = index.search(query);

    for(multimap<double, string>::iterator result = results.begin(); result != results.end(); ++result)
        cout << result->first << " " << result->second << endl;

    return 0;
}
