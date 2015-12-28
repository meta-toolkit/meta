/**
 * @file scorer.cpp
 * @author Chase Geigle
 */

#include "meta/sequence/crf/scorer.h"

namespace meta
{
namespace sequence
{

void crf::scorer::score(const crf& model, const sequence& seq)
{
    transition_scores(model);
    state_scores(model, seq);
    fwd_ = util::nullopt;
    bwd_ = util::nullopt;
    state_mrg_ = util::nullopt;
    trans_mrg_ = util::nullopt;
}

void crf::scorer::transition_scores(const crf& model)
{
    auto num_labels = model.num_labels();
    trans_.resize(num_labels, num_labels);
    trans_exp_.resize(num_labels, num_labels);
    for (label_id outer{0}; outer < num_labels; ++outer)
    {
        for (const auto& idx : model.trans_range(outer))
            trans_(outer, model.transition(idx)) = model.trans_weight(idx)
                                                   * model.scale_;

        // exponentiate and store in trans_exp_
        std::transform(trans_.begin(outer), trans_.end(outer),
                       trans_exp_.begin(outer), [](double val)
        { return std::exp(val); });
    }
}

void crf::scorer::state_scores(const crf& model, const sequence& seq)
{
    auto num_labels = model.num_labels();
    state_.resize(seq.size(), num_labels);
    state_exp_.resize(seq.size(), num_labels);
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        for (const auto& pair : seq[t].features())
        {
            auto value = model.scale_ * pair.second;
            for (const auto& idx : model.obs_range(pair.first))
            {
                auto lbl = model.observation(idx);
                state_(t, lbl) += model.obs_weight(idx) * value;
            }
        }

        // exponentiate and store in state_exp_
        std::transform(state_.begin(t), state_.end(t),
                       state_exp_.begin(t), [](double val)
        { return std::exp(val); });
    }
}

void crf::scorer::forward()
{
    fwd_ = forward_trellis{state_exp_.rows(), state_exp_.columns()};

    // initialize first column of trellis
    for (label_id lbl{0}; lbl < state_exp_.columns(); ++lbl)
        fwd_->probability(0, lbl, state_exp(0, lbl));
    // normalize to avoid underflow
    fwd_->normalize(0);

    // compute remaining columns of trellis using recursive formulation
    for (uint64_t t = 1; t < state_exp_.rows(); ++t)
    {
        for (label_id lbl{0}; lbl < state_exp_.columns(); ++lbl)
        {
            auto score = state_exp(t ,lbl);
            double sum = 0;
            for (label_id in{0}; in < state_exp_.columns(); ++in)
            {
                sum += fwd_->probability(t - 1, in) * trans_exp(in, lbl);
            }
            fwd_->probability(t, lbl, score * sum);
        }
        // normalize to avoid underflow
        fwd_->normalize(t);
    }
}

void crf::scorer::backward()
{
    if (!fwd_)
        forward();

    bwd_ = trellis{state_exp_.rows(), state_exp_.columns()};

    // initialize last column of the trellis
    for (label_id lbl{0}; lbl < state_exp_.columns(); ++lbl)
    {
        auto val = fwd_->normalizer(state_exp_.rows() - 1);
        bwd_->probability(state_exp_.rows() - 1, lbl, val);
    }

    // to avoid unsigned weirdness, t is really t+1, so we are actually
    // going to compute for index t-1 here to compute beta[t]
    for (uint64_t t = state_exp_.rows() - 1; t > 0; --t)
    {
        for (label_id i{0}; i < state_exp_.columns(); ++i)
        {
            double sum = 0;
            for (label_id j{0}; j < state_exp_.columns(); ++j)
            {
                sum += bwd_->probability(t, j) * state_exp(t, j)
                       * trans_exp(i, j);
            }
            bwd_->probability(t - 1, i, fwd_->normalizer(t - 1) * sum);
        }
    }
}

void crf::scorer::marginals()
{
    if (!fwd_)
        forward();

    if (!bwd_)
        backward();

    transition_marginals();
    state_marginals();
}

void crf::scorer::transition_marginals()
{
    trans_mrg_ = double_matrix{trans_exp_.rows(), trans_exp_.rows()};

    for (uint64_t t = 0; t < state_exp_.rows() - 1; ++t)
    {
        for (label_id lbl{0}; lbl < trans_mrg_->rows(); ++lbl)
        {
            for (label_id in{0}; in < trans_mrg_->columns(); ++in)
            {
                (*trans_mrg_)(lbl, in)
                    += fwd_->probability(t, lbl) * trans_exp(lbl, in)
                       * state_exp(t + 1, in) * bwd_->probability(t + 1, in);
            }
        }
    }
}

void crf::scorer::state_marginals()
{
    state_mrg_ = double_matrix{state_exp_.rows(), state_exp_.columns()};

    for (uint64_t t = 0; t < state_mrg_->rows(); ++t)
    {
        for (label_id lbl{0}; lbl < state_mrg_->columns(); ++lbl)
        {
            (*state_mrg_)(t, lbl) = fwd_->probability(t, lbl)
                                    * bwd_->probability(t, lbl)
                                    * (1.0 / fwd_->normalizer(t));
        }
    }
}

double crf::scorer::loss(const sequence& seq) const
{
    util::optional<label_id> prev;
    double score = 0;
    double normalizer = 0;
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        auto curr = seq[t].label();
        score += state(t, curr);
        if (prev)
            score += trans(*prev, curr);

        // factor in log normalizer: log(Z(x)) = - \sum_t log(scale[t])
        // and loss = -score(x) + log(Z(x)), so we add on log(scale[t])
        normalizer += std::log(fwd_->normalizer(t));
        prev = curr;
    }
    return -score - normalizer;
}

double crf::scorer::state(uint64_t time, label_id lbl) const
{
    return state_(time, lbl);
}

double crf::scorer::state_exp(uint64_t time, label_id lbl) const
{
    return state_exp_(time, lbl);
}

double crf::scorer::trans(label_id from, label_id to) const
{
    return trans_(from, to);
}

double crf::scorer::trans_exp(label_id from, label_id to) const
{
    return trans_exp_(from, to);
}

double crf::scorer::trans_marginal(label_id from, label_id to) const
{
    return (*trans_mrg_)(from, to);
}

double crf::scorer::state_marginal(uint64_t time, label_id lbl) const
{
    return (*state_mrg_)(time, lbl);
}

}
}
