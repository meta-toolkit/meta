#include <iostream>
#include <fstream>
#include "io/config_reader.h"

using std::ifstream;
using std::make_pair;
using std::string;
using std::unordered_map;
using std::cerr;
using std::endl;

unordered_map<string, string> ConfigReader::read(const string & path)
{
    unordered_map<string, string> options;
    ifstream configFile(path, ifstream::in);
    if(configFile.is_open())
    {
        string line;
        while(configFile.good())
        {
            std::getline(configFile, line);
            // skip comments and blank lines
            if(line.size() == 0 || line[0] == ';')
                continue;
            size_t spaceIndex = line.find(" ");
            string field = line.substr(0, spaceIndex);
            string value = line.substr(spaceIndex + 1, line.size());
            options.insert(make_pair(field, value));
        }        
        configFile.close();
    }
    else
    {
        cerr << "[ConfigReader]: Failed to open " << path << endl;
    }
    return options;
}
