/**
 * @file language_model.cpp
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/lm/language_model.h"
#include "meta/lm/read_arpa.h"
#include "meta/logging/logger.h"
#include "meta/util/fixed_heap.h"
#include "meta/util/shim.h"
#include "meta/util/time.h"

#include <random>
#include <sstream>

namespace meta
{
namespace lm
{

language_model::language_model(const cpptoml::table& config)
{
    auto table = config.get_table("language-model");
    auto arpa_file = table->get_as<std::string>("arpa-file");
    auto binary_file = table->get_as<std::string>("binary-file-prefix");

    N_ = 0;
    if (binary_file && filesystem::file_exists(*binary_file + "0.binlm"))
    {
        LOG(info) << "Loading language model from binary files: "
                  << *binary_file << "*" << ENDLG;
        auto time = common::time([&]() {
            prefix_ = *binary_file;
            load_vocab();
            while (filesystem::file_exists(*binary_file + std::to_string(N_)
                                           + ".binlm"))
                lm_.emplace_back(*binary_file + std::to_string(N_++)
                                 + ".binlm");
        });
        LOG(info) << "Done. (" << time.count() << "ms)" << ENDLG;
    }
    else if (arpa_file && binary_file)
    {
        LOG(info) << "Loading language model from .arpa file: " << *arpa_file
                  << ENDLG;
        prefix_ = *binary_file;
        auto time = common::time([&]() { read_arpa_format(*arpa_file); });
        LOG(info) << "Done. (" << time.count() << "ms)" << ENDLG;
    }
    else
        throw language_model_exception{
            "arpa-file or binary-file-prefix needed in config file"};

    // cache this value
    auto unk = vocabulary_.at("<unk>");
    unk_id_ = unk;
    unk_node_ = *lm_[0].find({unk_id_});
}

void language_model::read_arpa_format(const std::string& arpa_file)
{
    std::ifstream infile{arpa_file};
    std::vector<uint64_t> count;
    std::ofstream unigrams{prefix_ + "0.strings"};
    term_id unigram_id{0};

    read_arpa(
        infile, [&](uint64_t /* order */,
                    uint64_t ngramcount) { count.push_back(ngramcount); },
        [&](uint64_t order, const std::string& ngram, float prob,
            float backoff) {
            if (lm_.size() < order + 1)
            {
                lm_.emplace_back(prefix_ + std::to_string(N_) + ".binlm",
                                 count[N_]);
                ++N_;
            }

            if (order == 0)
            {
                unigrams << ngram << "\n";
                vocabulary_.emplace(ngram, unigram_id++);
            }

            lm_.back().insert(token_list{ngram, vocabulary_}, prob, backoff);
        });
}

std::vector<std::pair<std::string, float>>
language_model::top_k(const sentence& prev, size_t k) const
{
    // this is horribly inefficient due to this LM's structure
    using pair_t = std::pair<std::string, float>;
    auto comp
        = [](const pair_t& a, const pair_t& b) { return a.second > b.second; };
    util::fixed_heap<pair_t, decltype(comp)> candidates{k, comp};

    token_list candidate{prev, vocabulary_};
    candidate.push_back(0_tid);
    for (const auto& word : vocabulary_)
    {
        candidate[candidate.size() - 1] = word.second;
        candidates.emplace(word.first, log_prob(candidate));
    }

    return candidates.extract_top();
}

void language_model::load_vocab()
{
    std::string word;
    std::ifstream unigrams{prefix_ + "0.strings"};
    term_id cur{0};
    while (std::getline(unigrams, word))
    {
        if (word.empty())
            continue;

        vocabulary_.emplace(word, cur++);
    }
}

float language_model::log_prob(const sentence& tokens) const
{
    return log_prob(token_list{tokens, vocabulary_});
}

float language_model::log_prob(const token_list& tokens) const
{
    float prob = 0.0f;

    lm::lm_state state;
    lm::lm_state state_next;
    for (const auto& token : tokens.tokens())
    {
        prob += score(state, token, state_next);
        state = state_next;
    }

    return prob;
}

float language_model::perplexity(const sentence& tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{"perplexity() called on empty sentence"};
    return std::pow(10.0f, -(log_prob(tokens) / tokens.size()));
}

float language_model::perplexity_per_word(const sentence& tokens) const
{
    if (tokens.size() == 0)
        throw language_model_exception{
            "perplexity_per_word() called on empty sentence"};
    return perplexity(tokens) / tokens.size();
}

term_id language_model::index(const std::string& token) const
{
    auto it = vocabulary_.find(token);
    return it != vocabulary_.end() ? it->second : unk_id_;
}

term_id language_model::unk() const
{
    return unk_id_;
}

float language_model::score(const lm_state& in_state, term_id token,
                            lm_state& out_state) const
{
    out_state = in_state;
    out_state.previous.push_back(token);

    // (1) Find the longest matching ngram
    if (out_state.previous.size() == N_)
    {
        if (auto full = lm_[N_ - 1].find(out_state.previous))
        {
            out_state.shrink();
            return full->prob;
        }
        out_state.shrink();
    }

    float res = 0;
    while (out_state.previous.size() > 1)
    {
        const auto& table = lm_[out_state.previous.size() - 1];
        if (auto mid = table.find(out_state.previous))
        {
            res = mid->prob;
            break;
        }
        out_state.shrink();
    }

    if (out_state.previous.size() == 1)
    {
        auto uni_node = lm_[0].find(out_state.previous).value_or(unk_node_);
        res = uni_node.prob;
    }

    if (out_state.previous.size() > in_state.previous.size())
        return res;

    // (2) Apply backoff penalties if needed
    auto backoff = in_state;
    for (uint64_t i = 0;
         i < in_state.previous.size() - out_state.previous.size() + 1; ++i)
    {
        res += lm_[backoff.previous.size() - 1].find(backoff.previous)->backoff;
        backoff.shrink();
    }

    return res;
}
}
}
