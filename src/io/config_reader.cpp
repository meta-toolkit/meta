#include <fstream>
#include "tokenizers/tokenizers.h"

namespace meta {
namespace io {

using std::shared_ptr;
using std::make_shared;
using std::string;
using std::unordered_map;

using tokenizers::tokenizer;
using tokenizers::multi_tokenizer;
using tokenizers::tree_tokenizer;
using tokenizers::ngram_tokenizer;

string config_reader::get_config_string(const unordered_map<string, string> & config)
{
    std::stringstream ss;
    ss << "config";
    for(auto & opt: config)
    {
        // filter out unwanted (not useful) options
        if(opt.first != "slda" && opt.first != "liblinear" && opt.first != "prefix"
                && opt.first != "stop-words" && opt.first != "function-words")
            ss << "-" << opt.second;
    }
    return ss.str();
}

unordered_map<string, string> config_reader::read(const string & path)
{
    std::ifstream configFile(path);

    if(!configFile.is_open())
        throw config_reader_exception("failed to open " + path);

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
                opt += "_" + common::to_string(num_tokenizers);
            options[opt] = val;
        }
    }

    configFile.close();
    return options;
}

shared_ptr<tokenizer> config_reader::create_tokenizer(const unordered_map<string, string> & config)
{
    size_t current_tokenizer = 0;
    std::vector<shared_ptr<tokenizer>> toks;

    while(true)
    {
        string suffix = "_" + common::to_string(++current_tokenizer);
        auto method = config.find("method" + suffix);
        if(method == config.end())
            break;

        if(method->second == "tree")
        {
            string type = config.at("treeOpt" + suffix);
            if(type == "Branch")
                toks.emplace_back(make_shared<tokenizers::branch_tokenizer>());
            else if(type == "Depth")
                toks.emplace_back(make_shared<tokenizers::depth_tokenizer>());
            else if(type == "Semi")
                toks.emplace_back(make_shared<tokenizers::semi_skeleton_tokenizer>());
            else if(type == "Skel")
                toks.emplace_back(make_shared<tokenizers::skeleton_tokenizer>());
            else if(type == "Subtree")
                toks.emplace_back(make_shared<tokenizers::subtree_tokenizer>());
            else if(type == "Tag")
                toks.emplace_back(make_shared<tokenizers::tag_tokenizer>());
            else
                throw config_reader_exception{"tree method was not able to be determined"};
        }
        else if(method->second == "ngram")
        {
            int nVal;
            std::istringstream(config.at("ngram" + suffix)) >> nVal;
            string type = config.at("ngramOpt" + suffix);
            if(type == "Word")
                toks.emplace_back(shared_ptr<tokenizer>(new tokenizers::ngram_word_tokenizer<>(nVal)));
            else if(type == "FW")
                toks.emplace_back(shared_ptr<tokenizer>(new tokenizers::ngram_fw_tokenizer(nVal)));
            else if(type == "Lex")
                toks.emplace_back(shared_ptr<tokenizer>(new tokenizers::ngram_lex_tokenizer(nVal)));
            else if(type == "POS")
                toks.emplace_back(shared_ptr<tokenizer>(new tokenizers::ngram_pos_tokenizer(nVal)));
            else if(type == "Char")
                toks.emplace_back(shared_ptr<tokenizer>(new tokenizers::ngram_char_tokenizer(nVal)));
            else
                throw config_reader_exception("ngram method was not able to be determined");
        }
        else
            throw config_reader_exception("method was not able to be determined");
    }
    
    return shared_ptr<tokenizer>(new multi_tokenizer(toks));
}

}
}
