/**
 * @file hmm_train.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "cpptoml.h"
#include "meta/hashing/probe_map.h"
#include "meta/io/filesystem.h"
#include "meta/io/gzstream.h"
#include "meta/logging/logger.h"
#include "meta/sequence/hmm/discrete_observations.h"
#include "meta/sequence/hmm/hmm.h"
#include "meta/sequence/io/ptb_parser.h"
#include "meta/util/progress.h"

using namespace meta;

std::string two_digit(uint8_t num)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}
/**
 * Required config parameters:
 * ~~~toml
 * prefix = "global-data-prefix"
 *
 * [hmm]
 * prefix = "path-to-model"
 * treebank = "penn-treebank" # relative to data prefix
 * corpus = "wsj"
 * section-size = 99
 * train-sections = [0, 18]
 * dev-sections = [19, 21]
 * test-sections = [22, 24]
 * ~~~
 *
 * Optional config parameters: none
 */
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    auto prefix = config->get_as<std::string>("prefix");
    if (!prefix)
    {
        LOG(fatal) << "Global configuration must have a prefix key" << ENDLG;
        return 1;
    }

    auto seq_grp = config->get_table("hmm");
    if (!seq_grp)
    {
        LOG(fatal) << "Configuration must contain a [hmm] group" << ENDLG;
        return 1;
    }

    auto seq_prefix = seq_grp->get_as<std::string>("prefix");
    if (!seq_prefix)
    {
        LOG(fatal) << "[hmm] group must contain a prefix to store model files"
                   << ENDLG;
        return 1;
    }

    auto treebank = seq_grp->get_as<std::string>("treebank");
    if (!treebank)
    {
        LOG(fatal) << "[hmm] group must contain a treebank path" << ENDLG;
        return 1;
    }

    auto corpus = seq_grp->get_as<std::string>("corpus");
    if (!corpus)
    {
        LOG(fatal) << "[hmm] group must contain a corpus" << ENDLG;
        return 1;
    }

    auto train_sections = seq_grp->get_array("train-sections");
    if (!train_sections)
    {
        LOG(fatal) << "[hmm] group must contain train-sections" << ENDLG;
        return 1;
    }

    auto section_size = seq_grp->get_as<int64_t>("section-size");
    if (!section_size)
    {
        LOG(fatal) << "[hmm] group must contain section-size" << ENDLG;
        return 1;
    }

    std::string path
        = *prefix + "/" + *treebank + "/treebank-2/tagged/" + *corpus;

    hashing::probe_map<std::string, term_id> vocab;
    std::vector<std::vector<term_id>> training;
    {
        auto begin = train_sections->at(0)->as<int64_t>()->get();
        auto end = train_sections->at(1)->as<int64_t>()->get();
        printing::progress progress(
            " > Reading training data: ",
            static_cast<uint64_t>((end - begin + 1) * *section_size));
        for (auto i = static_cast<uint8_t>(begin); i <= end; ++i)
        {
            auto folder = two_digit(i);
            for (uint8_t j = 0; j <= *section_size; ++j)
            {
                progress(static_cast<uint64_t>(i - begin) * 99 + j);
                auto file = *corpus + "_" + folder + two_digit(j) + ".pos";
                auto filename = path + "/" + folder + "/" + file;
                auto sequences = sequence::extract_sequences(filename);
                for (auto& seq : sequences)
                {
                    std::vector<term_id> instance;
                    instance.reserve(seq.size());
                    for (const auto& obs : seq)
                    {
                        auto it = vocab.find(obs.symbol());
                        if (it == vocab.end())
                            it = vocab.insert(obs.symbol(),
                                              term_id{vocab.size()});
                        instance.push_back(it->value());
                    }
                    training.emplace_back(std::move(instance));
                }
            }
        }
    }

    using namespace sequence;
    using namespace hmm;

    std::mt19937 rng{47};
    discrete_observations<> obs_dist{
        30, vocab.size(), rng, stats::dirichlet<term_id>{1e-6, vocab.size()}};

    parallel::thread_pool pool;
    hidden_markov_model<discrete_observations<>> hmm{
        30, rng, std::move(obs_dist), stats::dirichlet<state_id>{1e-6, 30}};

    decltype(hmm)::training_options options;
    options.delta = 1e-5;
    options.max_iters = 50;
    hmm.fit(training, pool, options);

    filesystem::make_directories(*seq_prefix);
    {
        io::gzofstream file{*seq_prefix + "/model.gz"};
        hmm.save(file);
    }

    return 0;
}
