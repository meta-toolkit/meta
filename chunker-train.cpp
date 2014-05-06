#include <iostream>

#include "logging/logger.h"
#include "sequence/sequence.h"
#include "util/progress.h"
#include "sequence/analyzers/sequence_analyzer.h"
#include "sequence/crf/crf.h"
#include "sequence/io/conll_parser.h"
#include "util/filesystem.h"
#include "cpptoml.h"

using namespace meta;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
    {
        LOG(fatal) << "Global configuration must have a prefix key" << ENDLG;
        return 1;
    }

    auto crf_grp = config.get_group("chunker");
    if (!crf_grp)
    {
        LOG(fatal) << "Configuration must contain a [chunker] group" << ENDLG;
        return 1;
    }

    auto crf_prefix = crf_grp->get_as<std::string>("prefix");
    if (!crf_prefix)
    {
        LOG(fatal)
            << "[chunker] group must contain a prefix to store model files"
            << ENDLG;
        return 1;
    }

    auto dataset = crf_grp->get_as<std::string>("dataset");
    if (!dataset)
    {
        LOG(fatal) << "[chunker] group must contain a dataset path" << ENDLG;
        return 1;
    }

    // get POS-tagged sequences for analyzing
    auto data =
        sequence::conll::dataset(*prefix + "/" + *dataset + "/train.txt");

    filesystem::make_directory(*crf_prefix);
    auto& training = data.sequences();
    auto analyzer = sequence::default_chunking_analyzer(*crf_prefix);
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

    // replace the POS tags with BIO tags for training
    for (uint64_t i = 0; i < training.size(); ++i)
    {
        for (uint64_t j = 0; j < training[i].size(); ++j)
        {
            training[i][j].tag(data.tag(i, j));
        }
    }

    sequence::crf crf{*crf_prefix};
    crf.train({}, training);

    return 0;
}
