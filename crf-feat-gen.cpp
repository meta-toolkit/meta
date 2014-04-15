#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "sequence/io/ptb_parser.h"
#include "sequence/sequence.h"
#include "sequence/analyzers/sequence_analyzer.h"
#include "util/progress.h"
#include "util/filesystem.h"

std::string two_digit(uint8_t num)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}

std::string suffix(const std::string& input, uint64_t length)
{
    if (length > input.size())
        return input;
    return input.substr(input.size() - length);
}

std::string replace_all(std::string input, const std::string& search,
                        const std::string& replace)
{
    std::string::size_type pos = 0;
    while ((pos = input.find(search, pos)) != input.npos)
    {
        input.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return input;
}

std::string sanitize(std::string input)
{
    input = replace_all(std::move(input), "\\", "\\\\");
    input = replace_all(std::move(input), ":", "\\:");
    return input;
}

/**
 * @see
 * http://honnibal.wordpress.com/2013/09/11/a-good-part-of-speechpos-tagger-in-about-200-lines-of-python/
 */
void feature_gen(meta::sequence::sequence_analyzer& analyzer,
                 std::vector<meta::sequence::sequence>& seqs,
                 std::string filename)
{
    using namespace meta;

    std::ofstream output{filename};

    printing::progress progress{" > Generating features: ", seqs.size()};
    uint64_t idx = 0;
    for (auto& seq : seqs)
    {
        progress(++idx);
        analyzer.analyze(seq);
        for (uint64_t t = 0; t < seq.size(); ++t)
        {
            output << sanitize(seq[t].tag());
            for (const auto& pair : seq[t].features())
                output << "\t" << pair.first;
            output << "\n";
        }
        output << "\n";
    }
}

int main(int argc, char** argv)
{
    using namespace meta;
    if (argc < 4)
    {
        std::cout << "Usage: " << argv[0]
                  << " path-to-treebank train.txt test.txt" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    std::string path = argv[1];
    path += "/treebank-2/tagged/wsj";

    std::vector<sequence::sequence> training;
    std::vector<sequence::sequence> testing;

    {
        printing::progress progress{" > Reading training data: ", 18 * 99};
        for (uint8_t i = 0; i <= 18; ++i)
        {
            auto folder = two_digit(i);
            for (uint8_t j = 0; j <= 99; ++j)
            {
                progress(i * 99 + j);
                auto file = "wsj_" + folder + two_digit(j) + ".pos";
                auto filename = path + "/" + folder + "/" + file;
                auto sequences = sequence::extract_sequences(filename);
                for (auto & seq : sequences)
                    training.emplace_back(std::move(seq));
            }
        }
    }

    auto analyzer = sequence::default_pos_analyzer("analyzer.mapping");
    feature_gen(analyzer, training, argv[2]);
    training.clear();

    {
        printing::progress progress{" > Reading testing data: ", 3 * 99};
        for (uint8_t i = 19; i <= 21; ++i)
        {
            auto folder = two_digit(i);
            for (uint8_t j = 0; j <= 99; ++j)
            {
                progress((i - 19) * 99 + j);
                auto file = "wsj_" + folder + two_digit(j) + ".pos";
                auto filename = path + "/" + folder + "/" + file;
                auto sequences = sequence::extract_sequences(filename);
                for (auto & seq : sequences)
                    testing.emplace_back(std::move(seq));
            }
        }
    }

    feature_gen(analyzer, testing, argv[3]);
}
