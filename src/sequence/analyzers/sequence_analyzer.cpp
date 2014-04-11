/**
 * @file sequence_analyzer.cpp
 * @author Chase Geigle
 */

#include <sstream>
#include "sequence/analyzers/sequence_analyzer.h"

namespace meta
{
namespace sequence
{

void sequence_analyzer::analyze(sequence& sequence) const
{
    for (uint64_t t = 0; t < sequence.size(); ++t)
    {
        for (const auto& fn : obs_fns_)
            fn(sequence, t);
    }
}

namespace
{
std::string suffix(const std::string& input, uint64_t length)
{
    if (length > input.size())
        return input;
    return input.substr(input.size() - length);
}
}

sequence_analyzer default_pos_analyzer()
{
    // TODO: add more features, look at the MeLT paper for some
    sequence_analyzer analyzer;

    // current word features
    analyzer.add_observation_function([](sequence& seq, uint64_t t)
    {
        std::string word = seq[t].symbol();
        seq[t].increment("suffix=" + suffix(word, 3), 1);
        seq[t].increment(std::string{"prefix1="} + word[0], 1);
        seq[t].increment("w[t]=" + word, 1);
    });

    // previous word features
    analyzer.add_observation_function([](sequence& seq, uint64_t t)
    {
        std::string word = seq[t].symbol();
        auto prevword = std::string{"-START-"};
        auto prev2word = std::string{"-START2-"};
        if (t > 0)
        {
            prevword = seq[t-1].symbol();
            prev2word = "-START-";
        }
        if (t > 1)
            prev2word = seq[t-2].symbol();

        seq[t].increment("w[t-1]=" + prevword, 1);
        seq[t].increment("w[t-1]suffix=" + suffix(prevword, 3), 1);
        seq[t].increment("w[t-2]=" + prev2word, 1);
    });

    // next word features
    analyzer.add_observation_function([](sequence& seq, uint64_t t)
    {
        auto nextword = std::string{"-END-"};
        auto next2word = std::string{"-END2-"};
        if (t + 1 < seq.size())
        {
            nextword = seq[t+1].symbol();
            next2word = "-END-";
        }
        if (t + 2 < seq.size())
            next2word = seq[t+2].symbol();

        seq[t].increment("w[t+1]=" + nextword, 1);
        seq[t].increment("w[t+1]suffix=" + suffix(nextword, 3), 1);
        seq[t].increment("w[t+2]=" + next2word, 1);
    });

    return analyzer;
}

}
}
