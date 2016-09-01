/**
 * @file read_arpa.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LM_READ_ARPA_H_
#define META_LM_READ_ARPA_H_

#include <fstream>
#include <string>

#include "meta/config.h"

namespace meta
{
namespace lm
{

/**
 * Parses an ARPA formatted language model file.
 *
 * @param count_handler A callback function to be invoked when reading the
 * ngram count information from the file, of the form (order, count),
 * where order is **0-indexed**.
 * @param ngram_handler A callback function to be invoked when reading the
 * ngram statistics themselves, of the form (order, ngram, prob, backoff),
 * where order is **0-indexed**.
 */
template <class CountHandler, class NGramHandler>
void read_arpa(std::istream& infile, CountHandler&& count_handler,
               NGramHandler&& ngram_handler)
{
    std::string buffer;

    bool unigrams_found = false;
    bool counts_found = false;
    uint64_t order = 0;
    // get to beginning of unigram data, observing the counts of each ngram type
    while (std::getline(infile, buffer))
    {
        if (buffer.find("ngram ") == 0)
        {
            counts_found = true;
            auto equal = buffer.find_first_of("=");
            count_handler(order, std::stoul(buffer.substr(equal + 1)));
            ++order;
        }

        if (buffer.find("\\1-grams:") == 0)
        {
            unigrams_found = true;
            break;
        }
    }

    if (!unigrams_found || !counts_found)
        throw std::runtime_error{"invalid .arpa format"};

    order = 0;
    while (std::getline(infile, buffer))
    {
        // if blank or end
        if (buffer.empty() || (buffer[0] == '\\' && buffer[1] == 'e'))
            continue;

        // if start of new ngram data
        if (buffer[0] == '\\')
        {
            ++order;
            continue;
        }

        auto first_tab = buffer.find_first_of('\t');
        float prob = std::stof(buffer.substr(0, first_tab));
        auto second_tab = buffer.find_first_of('\t', first_tab + 1);
        auto ngram = buffer.substr(first_tab + 1, second_tab - first_tab - 1);
        float backoff = 0.0;
        if (second_tab != std::string::npos)
            backoff = std::stof(buffer.substr(second_tab + 1));

        ngram_handler(order, ngram, prob, backoff);
    }
}
}
}
#endif
