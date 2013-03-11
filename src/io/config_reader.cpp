#include <iostream>
#include <fstream>
#include "tokenizers/multi_tokenizer.h"
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

Tokenizer* ConfigReader::create_tokenizer(const unordered_map<string, string> & config)
{
    string method = config.at("method");
    int nVal;
    istringstream(config.at("ngram")) >> nVal;
     
    if(method == "ngram")
        return new NgramTokenizer(nVal, ngramOpt.at(config.at("ngramOpt")));
    else if(method == "tree")
        return new TreeTokenizer(treeOpt.at(config.at("treeOpt")));
    else if(method == "both")
        return new MultiTokenizer({
            new NgramTokenizer(nVal, ngramOpt.at(config.at("ngramOpt"))),
            new TreeTokenizer(treeOpt.at(config.at("treeOpt")))
        });

    cerr << "Method was not able to be determined" << endl;
    return nullptr;
}
