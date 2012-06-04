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

string collectionPrefix = "/home/sean/projects/senior-thesis-data/input/forums-collection/";
string queryPrefix = "/home/sean/projects/senior-thesis-data/input/forums-queries/";
string forums = "/home/sean/projects/senior-thesis-data/input/20news/";

vector<string> getFilenames(const string & filename)
{
    vector<string> files;
    ifstream inputFile(filename, ifstream::in);
    if(inputFile.is_open())
    {
        string line;
        while(inputFile.good())
        {
            std::getline(inputFile, line);
            if(line != "")
                files.push_back(forums + line);
        }        
        inputFile.close();
    }
    else
    {
        cerr << "Failed to open " << filename << endl;
    }
    return files;
}

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

/**
 * makeGreen - color a string green
 * @param str - the string to color
 */
inline string makeGreen(string str)
{
    return "\033[1;32m" + str + "\033[0m";
}

/**
 * makeRed - color a string red
 * @param str - the string to color
 */
inline string makeRed(string str)
{
    return "\033[1;31m" + str + "\033[0m";
}

int main(int argc, char* argv[])
{
    vector<string> indexFiles = getFilenames(forums + "20news.train");
    vector<string> queryFiles = getFilenames(forums + "20news.test.shortest");

    Tokenizer* tokenizer = new NgramTokenizer(2); 
    RAMIndex index(indexFiles, tokenizer);
    cout << "Running queries..." << endl;
    size_t numQueries = 1;
    size_t numCorrect = 0;
    for(vector<string>::iterator iter = queryFiles.begin(); iter != queryFiles.end(); ++iter)
    {
        size_t numResults = 0;
        string category = RAMIndex::getCategory(*iter);
        Document query(RAMIndex::getName(*iter), category);
        tokenizer->tokenize(*iter, query);
        string result = index.classifyKNN(query, 3);
        if(result == ( "(" + category + ")"))
        {
            ++numCorrect;
            cout << "  -> " << makeGreen("OK");
        }
        else
            cout << "  -> " << makeRed("incorrect");
        cout << " " << result << endl << "  -> " << ((double) numCorrect / numQueries * 100) << "% accuracy" << endl;
/*
        multimap<double, string> results = index.search(query);

        for(multimap<double, string>::reverse_iterator result = results.rbegin(); result !=
         results.rend() && numResults++ != 5; ++result)
            cout << " " << result->first << " " << result->second << endl;
*/
        ++numQueries;
    }

    delete tokenizer;
    return 0;
}
