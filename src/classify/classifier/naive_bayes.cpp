/**
 * @file naive_bayes.cpp
 * @author Sean Massung
 */

#include <cassert>
#include "cpptoml.h"
#include "meta/classify/classifier/naive_bayes.h"
#include "meta/io/packed.h"

namespace meta
{
namespace classify
{

const util::string_view naive_bayes::id = "naive-bayes";
const constexpr double naive_bayes::default_alpha;
const constexpr double naive_bayes::default_beta;

naive_bayes::naive_bayes(dataset_view_type docs, double alpha, double beta)
    : class_probs_{stats::dirichlet<class_label>{beta, docs.total_labels()}}
{
    stats::dirichlet<term_id> term_prior{alpha, docs.total_features()};

    std::vector<class_label> labels(docs.total_labels());

    std::transform(docs.labels_begin(), docs.labels_end(), labels.begin(),
                   [](const std::pair<const class_label, label_id>& pr)
                   {
                       return pr.first;
                   });
    std::sort(labels.begin(), labels.end());
    term_probs_.reserve(labels.size());
    for (const auto& lbl : labels)
        term_probs_.emplace_back(lbl, term_prior);

    train(docs);
}

naive_bayes::naive_bayes(std::istream& in)
{
    uint64_t size;
    auto bytes = io::packed::read(in, size);
    if (bytes == 0)
        throw naive_bayes_exception{
            "failed reading term probability file (no size written)"};

    term_probs_.clear();
    term_probs_.reserve(size);
    for (uint64_t i = 0; i < size; ++i)
    {
        class_label label;
        io::packed::read(in, label);
        term_probs_[label].load(in);
    }
    class_probs_.load(in);
}

void naive_bayes::save(std::ostream& os) const
{
    io::packed::write(os, id);

    io::packed::write(os, term_probs_.size());
    for (const auto& dist : term_probs_)
    {
        const auto& label = dist.first;
        const auto& probs = dist.second;
        io::packed::write(os, label);
        probs.save(os);
    }
    class_probs_.save(os);
}

void naive_bayes::train(const dataset_view_type& docs)
{
    for (const auto& instance : docs)
    {
        for (const auto& p : instance.weights)
        {
            term_probs_[docs.label(instance)].increment(p.first, p.second);
            assert(term_probs_[docs.label(instance)].probability(p.first) > 0);
        }
        class_probs_.increment(docs.label(instance), 1);
    }
}

class_label naive_bayes::classify(const feature_vector& instance) const
{
    class_label label;
    double best = std::numeric_limits<double>::lowest();

    // calculate prob of test doc for each class
    for (const auto& cls : term_probs_)
    {
        const auto& lbl = cls.first;
        const auto& term_dist = cls.second;

        double sum = 0.0;
        assert(class_probs_.probability(lbl) > 0);
        sum += std::log(class_probs_.probability(lbl));
        for (const auto& t : instance)
        {
            assert(term_dist.probability(t.first) > 0);
            sum += t.second * std::log(term_dist.probability(t.first));
        }

        if (sum > best)
        {
            best = sum;
            label = cls.first;
        }
    }

    return label;
}

template <>
std::unique_ptr<classifier>
    make_classifier<naive_bayes>(const cpptoml::table& config,
                                 multiclass_dataset_view training)
{
    auto alpha
        = config.get_as<double>("alpha").value_or(naive_bayes::default_alpha);

    auto beta
        = config.get_as<double>("beta").value_or(naive_bayes::default_beta);

    return make_unique<naive_bayes>(std::move(training), alpha, beta);
}
}
}
