#include <fstream>
#include <regex>

#include "logging/logger.h"
#include "sequence/io/ptb_parser.h"

namespace meta
{
namespace sequence
{

std::vector<sequence> extract_sequences(const std::string& filename)
{
    std::regex regex{"=====+\\s*$"};

    std::vector<sequence> results;
    std::ifstream file{filename};
    std::string line;
    sequence seq;
    while (std::getline(file, line))
    {
        if (seq.size() > 0)
        {
            if (std::regex_match(line, regex))
            {
                results.emplace_back(std::move(seq));
                continue;
            }

            if (line.length() == 0)
            {
                if (seq[seq.size() - 1].tag() == tag_t{"."})
                    results.emplace_back(std::move(seq));
                continue;
            }
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
