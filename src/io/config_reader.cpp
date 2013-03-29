#include <fstream>
#include "tokenizers/multi_tokenizer.h"
#include "io/config_reader.h"

namespace meta {
namespace io {

using std::shared_ptr;
using std::string;
using std::unordered_map;

using tokenizers::Tokenizer;
using tokenizers::MultiTokenizer;
using tokenizers::TreeTokenizer;
using tokenizers::NgramTokenizer;

unordered_map<string, string> ConfigReader::read(const string & path)
{
    std::ifstream configFile(path);

    if(!configFile.is_open())
        throw ConfigReaderException("failed to open " + path);

    unordered_map<string, string> options;
    size_t num_tokenizers = 0;
    bool in_tokenizer = false;
    string line;
    while(getline(configFile, line))
    {
        if(line == "" || line[0] == ';')
            continue;
        else if(line == "[general]")
            in_tokenizer = false;
        else if(line == "[tokenizer]")
        {
            ++num_tokenizers;
            in_tokenizer = true;
        }
        else
        {
            size_t space = line.find_first_of(' ');
            string opt = line.substr(0, space);
            string val = line.substr(space + 1);
            if(in_tokenizer)
                opt += "_" + common::toString(num_tokenizers);
            options[opt] = val;
        }
    }

    configFile.close();
    return options;
}

shared_ptr<Tokenizer> ConfigReader::create_tokenizer(const unordered_map<string, string> & config)
{
    size_t current_tokenizer = 0;
    std::vector<shared_ptr<Tokenizer>> toks;

    while(true)
    {
        string suffix = "_" + common::toString(++current_tokenizer);
        auto method = config.find("method" + suffix);
        if(method == config.end())
            break;

        if(method->second == "tree")
        {
            string tree = config.at("treeOpt" + suffix);
            toks.emplace_back(
                    shared_ptr<Tokenizer>(new TreeTokenizer(treeOpt.at(tree)))
            );
        }
        else if(method->second == "ngram")
        {
            int nVal;
            std::istringstream(config.at("ngram" + suffix)) >> nVal;
            string ngram = config.at("ngramOpt" + suffix);
            toks.emplace_back(
                    shared_ptr<Tokenizer>(new NgramTokenizer(nVal, ngramOpt.at(ngram)))
            );
        }
        else
            throw ConfigReaderException("method was not able to be determined");
    }
    
    return shared_ptr<Tokenizer>(new MultiTokenizer(toks));
}

}
}
