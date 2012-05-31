#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <map>

#include "document.h"
#include "parse_tree.h"
#include "engine.h"

using std::pair;
using std::make_pair;
using std::multimap;
using std::vector;
using std::cout;
using std::endl;
using std::string;

using namespace engine;

vector<Document> loadIndex()
{
    const string inputPath = "../../senior-thesis-data/input/";
    
    vector<string> filenames;
    filenames.push_back(inputPath + "one.tree");
    filenames.push_back(inputPath + "two.tree");
    filenames.push_back(inputPath + "three.tree");

    vector<Document> documents;

    // get a vector of parse trees for each file
    for(vector<string>::const_iterator file = filenames.begin(); file != filenames.end(); ++file)
    {
        vector<ParseTree> trees = util::getTrees(*file);
        Document document(*file, "N/A");
       
        // aggregate token counts for each tree 
        for(vector<ParseTree>::const_iterator tree = trees.begin(); tree != trees.end(); ++tree)
            util::tokenize(*tree, document);
        documents.push_back(document);
    }

    return documents;
}

void performSearch(Document query)
{
    vector<Document> documents = loadIndex();

    // score documents
    multimap<double, string> ranks;
    for(vector<Document>::const_iterator doc = documents.begin(); doc != documents.end(); ++doc)
    {
        double score = search::scoreDocument(*doc, query);
        ranks.insert(make_pair(score, doc->getAuthor()));
    }

    // display results
    int numToDisplay = 10;
    int displayed = 0;
    for(multimap<double, string>::const_reverse_iterator rank = ranks.rbegin();
        rank != ranks.rend() && displayed != numToDisplay; ++rank, ++displayed)
    {
        cout << rank->first << " " << rank->second << endl;
    }
}

int main(int argc, char* argv[])
{
    performSearch(Document("",""));
    return 0;
}
