/**
 * @file parser.cpp
 */

#include <unordered_set>
#include "io/parser.h"
#include "io/mmap_file.h"

#include <iostream>
using namespace std;

using std::string;
using std::vector;

Parser::Parser(const string & path, const string & delims):
    _idx(0),
    _filename(path),
    _tokens(vector<string>())
{
    std::unordered_set<char> invalid(delims.begin(), delims.end());

    MmapFile mmap_file(path);
    char* file = mmap_file.start();
    
    size_t left = 0;
    for(size_t i = 0; i < mmap_file.size(); ++i)
    {
        if(invalid.find(file[i]) != invalid.end())
        {
            // only create a string object once we know the exact size and
            // content
            if(left != i)
                _tokens.push_back(string(file + left, i - left));
            left = i + 1;
        }
    }
}

string Parser::filename() const
{
    return _filename;
}

string Parser::peek() const
{
    return _tokens.at(_idx);
}

string Parser::next()
{
    return _tokens[_idx++];
}

bool Parser::hasNext() const
{
    return _idx < _tokens.size();
}
