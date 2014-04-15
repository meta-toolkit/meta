/**
 * @file sequence_analyzer.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <fstream>
#include <sstream>
#include "io/binary.h"
#include "sequence/analyzers/sequence_analyzer.h"
#include "util/filesystem.h"
#include "util/mapping.h"
#include "util/progress.h"

namespace meta
{
namespace sequence
{

sequence_analyzer::sequence_analyzer(const std::string& prefix)
    : prefix_{prefix}
{
    filesystem::make_directory(prefix);
    load_feature_id_mapping();
    load_label_id_mapping();
}

void sequence_analyzer::load_feature_id_mapping()
{
    if (!filesystem::file_exists(prefix_ + "/feature.mapping"))
        return;

    std::ifstream input{prefix_ + "/feature.mapping", std::ios::binary};
    if (!input)
        return;

    uint64_t num_keys;
    io::read_binary(input, num_keys);
    printing::progress progress{" > Loading feature mapping: ", num_keys};
    num_keys = 0;
    while (input)
    {
        progress(++num_keys);
        std::string key;
        feature_id value;
        io::read_binary(input, key);
        io::read_binary(input, value);
        feature_id_mapping_[key] = value;
    }
}

void sequence_analyzer::load_label_id_mapping()
{
    if (!filesystem::file_exists(prefix_ + "/label.mapping"))
        return;

    map::load_mapping(label_id_mapping_, prefix_ + "/label.mapping");
}

sequence_analyzer::~sequence_analyzer()
{
    save();
}

void sequence_analyzer::save()
{
    printing::progress progress{" > Saving feature mapping: ",
                                feature_id_mapping_.size()};
    std::ofstream output{prefix_, std::ios::binary};
    uint64_t i = 0;
    for (const auto& pair : feature_id_mapping_)
    {
        progress(++i);
        io::write_binary(output, pair.first);
        io::write_binary(output, pair.second);
    }
    map::save_mapping(label_id_mapping_, prefix_ + "/label.mapping");
}

void sequence_analyzer::analyze(sequence& sequence)
{
    for (uint64_t t = 0; t < sequence.size(); ++t)
    {
        collector coll{this, &sequence[t]};
        for (const auto& fn : obs_fns_)
            fn(sequence, t, coll);
    }
}

feature_id sequence_analyzer::feature(const std::string& feature)
{
    auto it = feature_id_mapping_.find(feature);
    if (it != feature_id_mapping_.end())
        return it->second;
    auto sze = feature_id_mapping_.size();
    feature_id_mapping_[feature] = sze;
    return feature_id{sze};
}

uint64_t sequence_analyzer::num_features() const
{
    return feature_id_mapping_.size();
}

sequence_analyzer::collector::collector(sequence_analyzer* analyzer,
                                        observation* obs)
    : analyzer_{analyzer}, obs_{obs}
{
    // nothing
}

sequence_analyzer::collector::~collector()
{
    using pair = std::pair<feature_id, double>;
    std::sort(feats_.begin(), feats_.end(), [](const pair& lhs, const pair& rhs)
    { return lhs.first < rhs.first; });

    obs_->features(std::move(feats_));
}

const std::string& sequence_analyzer::prefix() const
{
    return prefix_;
}

void sequence_analyzer::collector::add(const std::string& feature,
                                       double amount)
{
    auto fid = analyzer_->feature(feature);
    feats_.emplace_back(fid, amount);
}

label_id sequence_analyzer::label(class_label lbl) const
{
    return label_id_mapping_.get_value(lbl);
}

class_label sequence_analyzer::label(label_id lbl) const
{
    return label_id_mapping_.get_key(lbl);
}

uint64_t sequence_analyzer::num_labels() const
{
    return label_id_mapping_.size();
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

sequence_analyzer default_pos_analyzer(const std::string& prefix)
{
    // TODO: add more features, look at the MeLT paper for some
    sequence_analyzer analyzer{prefix};

    // current word features
    analyzer.add_observation_function([](sequence& seq, uint64_t t,
                                         sequence_analyzer::collector& coll)
    {
        std::string word = seq[t].symbol();
        coll.add("suffix=" + suffix(word, 3), 1);
        coll.add(std::string{"prefix1="} + word[0], 1);
        coll.add("w[t]=" + word, 1);
    });

    // previous word features
    analyzer.add_observation_function([](sequence& seq, uint64_t t,
                                         sequence_analyzer::collector& coll)
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

        coll.add("w[t-1]=" + prevword, 1);
        coll.add("w[t-1]suffix=" + suffix(prevword, 3), 1);
        coll.add("w[t-2]=" + prev2word, 1);
    });

    // next word features
    analyzer.add_observation_function([](sequence& seq, uint64_t t,
                                         sequence_analyzer::collector& coll)
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

        coll.add("w[t+1]=" + nextword, 1);
        coll.add("w[t+1]suffix=" + suffix(nextword, 3), 1);
        coll.add("w[t+2]=" + next2word, 1);
    });

    return analyzer;
}

}
}
