/**
 * @file naive_bayes.cpp
 * @author Sean Massung
 */

#include <cassert>
#include <unordered_set>
#include "cpptoml.h"
#include "classify/classifier/naive_bayes.h"
#include "index/postings_data.h"
#if META_HAS_ZLIB
#include "io/gzstream.h"
#endif
#include "io/binary.h"
#include "io/packed.h"

namespace meta
{
namespace classify
{

const util::string_view naive_bayes::id = "naive-bayes";
const constexpr double naive_bayes::default_alpha;
const constexpr double naive_bayes::default_beta;

naive_bayes::naive_bayes(std::shared_ptr<index::forward_index> idx,
                         double alpha, double beta)
    : classifier{std::move(idx)},
      class_probs_{stats::dirichlet<class_label>{beta, idx_->num_labels()}}
{
    stats::dirichlet<term_id> term_prior{alpha, idx_->unique_terms()};
    auto lbls = idx_->class_labels();
    std::sort(std::begin(lbls), std::end(lbls));
    term_probs_.reserve(lbls.size());
    for (const auto& lbl : lbls)
        term_probs_.emplace_back(lbl, term_prior);
}

void naive_bayes::reset()
{
    for (auto& term_dist : term_probs_)
        term_dist.second.clear();
    class_probs_.clear();
}

void naive_bayes::train(const std::vector<doc_id>& docs)
{
    for (auto& d_id : docs)
    {
        auto pdata = idx_->search_primary(d_id);
        auto lbl = idx_->label(d_id);
        for (auto& p : pdata->counts())
        {
            term_probs_[lbl].increment(p.first, p.second);
            assert(term_probs_[lbl].probability(p.first) > 0);
        }
        class_probs_.increment(lbl, 1);
    }
}

class_label naive_bayes::classify(doc_id d_id)
{
    class_label label;
    double best = std::numeric_limits<double>::lowest();

    // calculate prob of test doc for each class
    for (auto& cls : term_probs_)
    {
        const auto& lbl = cls.first;
        const auto& term_dist = cls.second;

        double sum = 0.0;
        assert(class_probs_.probability(lbl) > 0);
        sum += log(class_probs_.probability(lbl));
        auto pdata = idx_->search_primary(d_id);
        for (auto& t : pdata->counts())
        {
            assert(term_dist.probability(t.first) > 0);
            sum += t.second * log(term_dist.probability(t.first));
        }

        if (sum > best)
        {
            best = sum;
            label = cls.first;
        }
    }

    return label;
}

void naive_bayes::save(const std::string& prefix) const
{
#if META_HAS_ZLIB
    io::gzofstream tp_out{prefix + "/nb-term-probs.gz"};
    io::gzofstream cp_out{prefix + "/nb-class-probs.gz"};
#else
    std::ofstream tp_out{prefix + "/nb-term-probs", std::ios::binary};
    std::ofstream cp_out{prefix + "/nb-class-probs", std::ios::binary};
#endif

    io::packed::write(tp_out, term_probs_.size());
    for (const auto& dist : term_probs_)
    {
        const auto& label = dist.first;
        const auto& probs = dist.second;
        io::write_binary(tp_out, static_cast<std::string>(label));
        probs.save(tp_out);
    }
    class_probs_.save(cp_out);
}

void naive_bayes::load(const std::string& prefix)
{
#if META_HAS_ZLIB
    io::gzifstream tp_in{prefix + "/nb-term-probs.gz"};
    io::gzifstream cp_in{prefix + "/nb-class-probs.gz"};
#else
    std::ifstream tp_in{prefix + "/nb-term-probs", std::ios::binary};
    std::ifstream cp_in{prefix + "/nb-class-probs", std::ios::binary};
#endif

    if (!tp_in)
        throw naive_bayes_exception{"term probability file not found at prefix "
                                    + prefix};

    if (!cp_in)
        throw naive_bayes_exception{
            "class probability file not found at prefix " + prefix};

    uint64_t size;
    auto bytes = io::packed::read(tp_in, size);
    if (bytes == 0)
        throw naive_bayes_exception{
            "failed reading term probability file (no size written)"};

    term_probs_.clear();
    term_probs_.reserve(size);
    for (uint64_t i = 0; i < size; ++i)
    {
        std::string label;
        io::read_binary(tp_in, label);
        term_probs_[class_label{label}].load(tp_in);
    }
    class_probs_.load(cp_in);
}

template <>
std::unique_ptr<classifier>
    make_classifier<naive_bayes>(const cpptoml::table& config,
                                 std::shared_ptr<index::forward_index> idx)
{
    auto alpha
        = config.get_as<double>("alpha").value_or(naive_bayes::default_alpha);

    auto beta
        = config.get_as<double>("beta").value_or(naive_bayes::default_beta);

    return make_unique<naive_bayes>(std::move(idx), alpha, beta);
}
}
}
