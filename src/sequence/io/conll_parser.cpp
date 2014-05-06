/**
 * @file conll_parser.cpp
 * @author Chase Geigle
 */

#include <fstream>

#include "logging/logger.h"
#include "sequence/io/conll_parser.h"

namespace meta
{
namespace sequence
{
namespace conll
{

dataset::dataset(const std::string& filename)
{
    std::ifstream file{filename};
    std::string line;
    sequence seq;
    std::vector<tag_t> tags;
    while (std::getline(file, line))
    {
        if (line.empty() && seq.size() > 0)
        {
            sequences_.emplace_back(std::move(seq));
            tags_.emplace_back(std::move(tags));
            continue;
        }

        std::stringstream ss{line};
        std::string word;
        std::string pos;
        std::string tag;
        ss >> word >> pos >> tag;

        seq.add_observation({symbol_t{word}, tag_t{pos}});
        tags.emplace_back(tag_t{tag});
    }

    // handle any remaining sequences
    if (seq.size() > 0)
    {
        sequences_.emplace_back(std::move(seq));
        tags_.emplace_back(std::move(tags));
    }
}

std::vector<sequence>& dataset::sequences()
{
    return sequences_;
}

const tag_t& dataset::tag(uint64_t seq, uint64_t obs) const
{
    return tags_.at(seq).at(obs);
}
}
}
}
