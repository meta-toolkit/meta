/**
 * @file perceptron.cpp
 * @author Chase Geigle
 */

#include <fstream>

#include "meta/io/filesystem.h"
#include "meta/io/gzstream.h"
#include "meta/sequence/perceptron.h"
#include "meta/utf/utf.h"
#include "meta/util/progress.h"
#include "meta/util/time.h"

namespace meta
{
namespace sequence
{

perceptron::perceptron() : analyzer_{default_pos_analyzer()}
{
    // special analyzer function that adds features that wouldn't be
    // allowed under our linear-chain CRF model
    analyzer_.add_observation_function(
        [](const sequence& seq, uint64_t t, sequence_analyzer::collector& coll)
        {
            std::string prev = "<s>";
            std::string prev2 = "<s1>";
            if (t > 0)
            {
                prev = std::to_string(seq[t - 1].label());

                if (t > 1)
                    prev2 = std::to_string(seq[t - 2].label());
                else
                    prev2 = "<s>";
            }

            coll.add("q[t-2]=" + prev2, 1);
            coll.add("q[t-1]=" + prev, 1);
            coll.add("q[t-2]q[t-1]=" + prev2 + "-" + prev, 1);
            coll.add(
                "q[t-1]w[t]=" + prev + "-" + utf::foldcase(seq[t].symbol()), 1);
        });
}

perceptron::perceptron(const std::string& prefix) : perceptron()
{
    analyzer_.load(prefix);
    io::gzifstream file{prefix + "/tagger.model.gz"};
    model_.load(file);
}

void perceptron::tag(sequence& seq) const
{
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        analyzer_.analyze(seq, t);
        seq[t].label(model_.best_class(seq[t].features()));
        seq[t].tag(analyzer_.tag(seq[t].label()));
    }
}

void perceptron::train(std::vector<sequence>& sequences,
                       const training_options& options)
{
    std::default_random_engine rng{options.seed};

    std::vector<size_t> indices(sequences.size());
    std::iota(indices.begin(), indices.end(), 0);

    classify::linear_model<feature_id, double, label_id> for_avg;
    uint64_t total_updates = 0;
    for (uint64_t epoch = 1; epoch <= options.max_iterations; ++epoch)
    {
        std::shuffle(indices.begin(), indices.end(), rng);

        double num_correct = 0;
        double num_incorrect = 0;

        auto time = common::time(
            [&]()
            {
                printing::progress progress{" > Iteration "
                                            + std::to_string(epoch) + ": ",
                                            sequences.size()};
                for (uint64_t i = 0; i < indices.size(); ++i)
                {
                    progress(i);
                    auto& seq = sequences[indices[i]];

                    for (uint64_t t = 0; t < seq.size(); ++t)
                    {
                        analyzer_.analyze(seq, t);

                        auto lbl = model_.best_class(seq[t].features());
                        auto correct = analyzer_.label(seq[t].tag());

                        ++total_updates;
                        if (lbl != correct)
                        {
                            ++num_incorrect;
                            for (const auto& feat : seq[t].features())
                            {
                                model_.update(lbl, feat.first,
                                              -1.0 * feat.second);
                                for_avg.update(lbl, feat.first,
                                               -1.0 * (total_updates - 1)
                                               * feat.second);

                                model_.update(correct, feat.first, feat.second);
                                for_avg.update(correct, feat.first,
                                               (total_updates - 1)
                                               * feat.second);
                            }
                        }
                        else
                        {
                            ++num_correct;
                        }
                    }
                }
            });

        LOG(info) << "Took " << time.count() / 1000.0 << "s" << ENDLG;
        LOG(info) << "Training accuracy: "
                  << num_correct / (num_correct + num_incorrect) * 100 << "%"
                  << ENDLG;

        model_.condense(true);
        for_avg.condense(false);
    }

    // update weights to be average over all parameters
    model_.update(for_avg.weights(), -1.0 / total_updates);
}

void perceptron::save(const std::string& prefix) const
{
    analyzer_.save(prefix);
    io::gzofstream file{prefix + "/tagger.model.gz"};
    model_.save(file);
}
}
}
