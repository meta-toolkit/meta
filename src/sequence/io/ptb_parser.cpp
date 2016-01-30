#include <fstream>

#include "meta/logging/logger.h"
#include "meta/sequence/io/ptb_parser.h"

namespace meta
{
namespace sequence
{

namespace
{
bool is_paragraph_divider(const std::string& str)
{
    auto pos = str.find("=====");
    if (pos != 0)
        return false;
    if (str[str.size() - 1] != '=')
        return false;
    return true;
}
}

std::vector<sequence> extract_sequences(const std::string& filename)
{
    std::vector<sequence> results;
    std::ifstream file{filename};
    std::string line;
    sequence seq;
    while (std::getline(file, line))
    {
        // blank line
        if (line.length() == 0)
        {
            if (seq.size() > 0)
            {
                if (seq[seq.size() - 1].tag() == tag_t{"."})
                    results.emplace_back(std::move(seq));
            }
            continue;
        }

        // paragraph divider
        if (is_paragraph_divider(line))
        {
            if (seq.size() > 0)
                results.emplace_back(std::move(seq));
            continue;
        }

        std::stringstream ss{line};
        std::string word;
        while (ss >> word)
        {
            if (word == "]" || word == "[")
                continue;
            auto pos = word.rfind('/');
            if (pos == word.npos)
            {
                LOG(warning) << "could not find '/' in word/tag pair" << ENDLG;
                LOG(warning) << "word/tag pair is: " << word << ENDLG;
                continue;
            }
            auto sym = symbol_t{word.substr(0, pos)};
            auto tag = tag_t{word.substr(pos + 1)};
            seq.add_observation({sym, tag});
        }
    }
    if (seq.size() > 0)
        results.emplace_back(std::move(seq));

    return results;
}

}
}
