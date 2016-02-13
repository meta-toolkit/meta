/**
 * @file sequence_analyzer.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <fstream>
#include <sstream>
#include "meta/io/packed.h"
#include "meta/io/filesystem.h"
#include "meta/io/gzstream.h"
#include "meta/sequence/sequence_analyzer.h"
#include "meta/utf/utf.h"
#include "meta/util/mapping.h"
#include "meta/util/progress.h"

namespace meta
{
namespace sequence
{

sequence_analyzer::sequence_analyzer(const std::string& prefix)
{
    load(prefix);
}

void sequence_analyzer::load(const std::string& prefix)
{
    feature_id_mapping_.clear();
    label_id_mapping_.clear();
    load_feature_id_mapping(prefix);
    load_label_id_mapping(prefix);
}

void sequence_analyzer::load_feature_id_mapping(const std::string& prefix)
{
    auto feature_file = prefix + "/feature.mapping.gz";
    if (!filesystem::file_exists(feature_file))
        throw exception{"missing feature id mapping: " + feature_file};

    io::gzifstream input{feature_file};

    uint64_t total_num_keys;
    io::packed::read(input, total_num_keys);
    printing::progress progress{" > Loading feature mapping: ", total_num_keys};

    for (uint64_t num_keys = 0; num_keys < total_num_keys; ++num_keys)
    {
        progress(num_keys);
        std::string key;
        feature_id value;
        io::packed::read(input, key);
        io::packed::read(input, value);
        feature_id_mapping_[key] = value;
    }
}

void sequence_analyzer::load_label_id_mapping(const std::string& prefix)
{
    auto label_file = prefix + "/label.mapping";
    if (!filesystem::file_exists(label_file))
        throw exception{"missing label mapping: " + label_file};

    map::load_mapping(label_id_mapping_, label_file);
}

void sequence_analyzer::save(const std::string& prefix) const
{
    printing::progress progress{" > Saving feature mapping: ",
                                feature_id_mapping_.size()};

    io::gzofstream output{prefix + "/feature.mapping.gz"};
    io::packed::write(output, feature_id_mapping_.size());
    uint64_t i = 0;
    for (const auto& pair : feature_id_mapping_)
    {
        progress(++i);
        io::packed::write(output, pair.first);
        io::packed::write(output, pair.second);
    }
    map::save_mapping(label_id_mapping_, prefix + "/label.mapping");
}

void sequence_analyzer::analyze(sequence& sequence)
{
    for (uint64_t t = 0; t < sequence.size(); ++t)
        analyze(sequence, t);
}

void sequence_analyzer::analyze(sequence& sequence, uint64_t t)
{
    default_collector coll{this, &sequence[t]};
    for (const auto& fn : obs_fns_)
        fn(sequence, t, coll);
    if (!label_id_mapping_.contains_key(sequence[t].tag()))
    {
        label_id id(static_cast<uint32_t>(label_id_mapping_.size()));
        label_id_mapping_.insert(sequence[t].tag(), id);
    }
    sequence[t].label(label_id_mapping_.get_value(sequence[t].tag()));
}

void sequence_analyzer::analyze(sequence& sequence) const
{
    for (uint64_t t = 0; t < sequence.size(); ++t)
        analyze(sequence, t);
}

void sequence_analyzer::analyze(sequence& sequence, uint64_t t) const
{
    const_collector coll{this, &sequence[t]};
    for (const auto& fn : obs_fns_)
        fn(sequence, t, coll);

    if (!sequence[t].tagged()
        || !label_id_mapping_.contains_key(sequence[t].tag()))
        sequence[t].label(
            label_id(static_cast<uint32_t>(label_id_mapping_.size())));
    else
        sequence[t].label(label_id_mapping_.get_value(sequence[t].tag()));
}

feature_id sequence_analyzer::feature(const std::string& feature)
{
    auto it = feature_id_mapping_.find(feature);
    if (it != feature_id_mapping_.end())
        return it->second;
    auto sze = feature_id{feature_id_mapping_.size()};
    feature_id_mapping_[feature] = sze;
    return sze;
}

feature_id sequence_analyzer::feature(const std::string& feature) const
{
    auto it = feature_id_mapping_.find(feature);
    if (it != feature_id_mapping_.end())
        return it->second;
    return feature_id{feature_id_mapping_.size()};
}

uint64_t sequence_analyzer::num_features() const
{
    return feature_id_mapping_.size();
}

const util::invertible_map<tag_t, label_id>& sequence_analyzer::labels() const
{
    return label_id_mapping_;
}

label_id sequence_analyzer::label(tag_t lbl) const
{
    return label_id_mapping_.get_value(lbl);
}

tag_t sequence_analyzer::tag(label_id lbl) const
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

std::string prefix(const std::string& input, uint64_t length)
{
    using diff_type = std::string::iterator::difference_type;
    if (length > input.size())
        return {input.begin(), input.end()};
    return {input.begin(), input.begin() + static_cast<diff_type>(length)};
}
}

sequence_analyzer default_pos_analyzer()
{
    sequence_analyzer analyzer;

    auto word_feats = [](const std::string& word, uint64_t t,
                         sequence_analyzer::collector& coll)
    {
        auto norm = utf::foldcase(word);
        for (uint64_t i = 1; i <= 4; i++)
        {
            auto len = std::to_string(i);
            coll.add("w[t]_suffix_" + len + "=" + suffix(norm, i), 1);
            coll.add("w[t]_prefix_" + len + "=" + prefix(norm, i), 1);
        }
        coll.add("w[t]=" + norm, 1);

        // additional binary word features
        if (std::any_of(word.begin(), word.end(), [](char c)
                        {
                            return std::isdigit(c);
                        }))
        {
            coll.add("w[t]_has_digit=1", 1);
        }

        if (std::find(word.begin(), word.end(), '-') != word.end())
            coll.add("w[t]_has_hyphen=1", 1);

        if (std::any_of(word.begin(), word.end(), [](char c)
                        {
                            return std::isupper(c);
                        }))
        {
            coll.add("w[t]_has_upper=1", 1);
            if (t != 0)
            {
                coll.add("w[t]_has_upper_and_not_sentence_start=1", 1);
            }
        }

        if (std::all_of(word.begin(), word.end(), [](char c)
                        {
                            return std::isupper(c);
                        }))
        {
            coll.add("w[t]_all_upper=1", 1);
        }
    };

    // current word features
    analyzer.add_observation_function(
        [=](const sequence& seq, uint64_t t, sequence_analyzer::collector& coll)
        {
            std::string word = seq[t].symbol();
            word_feats(word, t, coll);
        });

    // previous word features
    analyzer.add_observation_function(
        [](const sequence& seq, uint64_t t, sequence_analyzer::collector& coll)
        {
            std::string word = seq[t].symbol();
            if (t > 0)
            {
                auto prevword = seq[t - 1].symbol();
                coll.add("w[t-1]=" + utf::foldcase(prevword), 1);
                if (t > 1)
                {
                    auto prev2word = seq[t - 2].symbol();
                    coll.add("w[t-2]=" + utf::foldcase(prev2word), 1);
                }
                else
                {
                    coll.add("w[t-2]=<s>", 1);
                }
            }
            else
            {
                coll.add("w[t-1]=<s>", 1);
                coll.add("w[t-2]=<s1>", 1);
            }
        });

    // next word features
    analyzer.add_observation_function(
        [](const sequence& seq, uint64_t t, sequence_analyzer::collector& coll)
        {
            if (t + 1 < seq.size())
            {
                auto nextword = seq[t + 1].symbol();
                coll.add("w[t+1]=" + utf::foldcase(nextword), 1);
                if (t + 2 < seq.size())
                {
                    auto next2word = seq[t + 2].symbol();
                    coll.add("w[t+2]=" + utf::foldcase(next2word), 1);
                }
                else
                {
                    coll.add("w[t+2]=</s>", 1);
                }
            }
            else
            {
                coll.add("w[t+1]=</s>", 1);
                coll.add("w[t+2]=</s1>", 1);
            }
        });

    // bias term
    analyzer.add_observation_function(
        [](const sequence&, uint64_t, sequence_analyzer::collector& coll)
        {
            coll.add("bias", 1);
        });

    return analyzer;
}
}
}
