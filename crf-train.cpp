#include <iostream>

#include "logging/logger.h"
#include "sequence/sequence.h"
#include "util/progress.h"
#include "sequence/analyzers/sequence_analyzer.h"
#include "sequence/crf.h"
#include "sequence/io/ptb_parser.h"
#include "util/filesystem.h"

using namespace meta;

std::string two_digit(uint8_t num)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " path-to-treebank" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    std::string path = argv[1];
    path += "/treebank-2/tagged/wsj";

    std::vector<sequence::sequence> training;
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

    filesystem::make_directory("crf");
    auto analyzer = sequence::default_pos_analyzer("crf");

    {
        printing::progress progress{" > Generating features: ",
                                    training.size()};
        uint64_t idx = 0;
        for (auto& seq : training)
        {
            progress(++idx);
            analyzer.analyze(seq);
        }
    }
    analyzer.save();

    sequence::crf crf{analyzer};
    crf.train({}, training);

    return 0;
}
