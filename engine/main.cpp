#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <map>

#include "tokenizer.h"
#include "ngram_tokenizer.h"
#include "pos_tree_tokenizer.h"
#include "parse_tree.h"
#include "ram_index.h"
#include "search.h"
#include "document.h"

using std::pair;
using std::make_pair;
using std::multimap;
using std::vector;
using std::cout;
using std::endl;
using std::string;

string collectionPrefix = "/home/sean/projects/senior-thesis-data/input/news-collection/";
string queryPrefix = "/home/sean/projects/senior-thesis-data/input/news-queries/";

/**
 * from
 * http://www.linuxquestions.org/questions/programming-9/c-list-files-in-directory-379323/
 */
vector<string> getdir(string dir)
{
    DIR *dp;
    vector<string> files;
    struct dirent *dirp;
    if((dp = opendir(dir.c_str())) == NULL)
    {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return files;
    }

    while((dirp = readdir(dp)) != NULL)
    {
        string name = string(dirp->d_name);
        if(name != "." && name != "..")
            files.push_back(dir + name);
    }
    closedir(dp);
    return files;
}

int main(int argc, char* argv[])
{
    vector<string> indexFiles = getdir(collectionPrefix);
    vector<string> queryFiles = getdir(queryPrefix);

    Tokenizer* tokenizer = new NgramTokenizer(1); 
    RAMIndex index(indexFiles, tokenizer);
    Document query(queryFiles[0], "nationality");
    tokenizer->tokenize(queryFiles[0], query);
    multimap<double, string> results = index.search(query);

    for(multimap<double, string>::reverse_iterator result = results.rbegin(); result != results.rend(); ++result)
        cout << " " << result->first << " " << result->second << endl;

    delete tokenizer;
    return 0;
}
