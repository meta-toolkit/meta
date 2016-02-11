/**
 * @file word_embeddings.cpp
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/embeddings/word_embeddings.h"
#include "meta/io/packed.h"
#include "meta/math/vector.h"
#include "meta/util/fixed_heap.h"
#include "meta/util/progress.h"

namespace meta
{
namespace embeddings
{

using vocab_type = hashing::probe_map<util::string_view, std::size_t>;

word_embeddings::word_embeddings(std::istream& vocab, std::istream& vectors)
    : vector_size_{io::packed::read<std::size_t>(vectors)},
      id_to_term_(io::packed::read<std::size_t>(vocab)),
      term_to_id_{static_cast<std::size_t>(std::ceil(
          id_to_term_.size() / vocab_type::default_max_load_factor()))},
      embeddings_(vector_size_ * (id_to_term_.size() + 1))
{
    load_vocab(vocab);

    printing::progress progress{" > Loading embeddings: ", id_to_term_.size()};
    // +1 for the unk vector, which is always last
    for (std::size_t tid = 0; tid < id_to_term_.size() + 1; ++tid)
    {
        if (!vectors)
            throw word_embeddings_exception{
                "embeddings stream ended unexpectedly"};

        progress(tid);
        auto vec = vector(tid);
        std::generate(vec.begin(), vec.end(), [&]()
                      {
                          return io::packed::read<double>(vectors);
                      });
    }
}

word_embeddings::word_embeddings(std::istream& vocab, std::istream& first,
                                 std::istream& second)
    : vector_size_{io::packed::read<std::size_t>(first)},
      id_to_term_(io::packed::read<std::size_t>(vocab)),
      term_to_id_{static_cast<std::size_t>(std::ceil(
          id_to_term_.size() / vocab_type::default_max_load_factor()))},
      embeddings_(vector_size_ * (id_to_term_.size() + 1))
{
    if (io::packed::read<std::size_t>(second) != vector_size_)
        throw word_embeddings_exception{"mismatched vector sizes"};

    load_vocab(vocab);

    printing::progress progress{" > Loading embeddings: ", id_to_term_.size()};
    // +1 for the unk vector, which is always last
    for (std::size_t tid = 0; tid < id_to_term_.size() + 1; ++tid)
    {
        if (!first)
            throw word_embeddings_exception{
                "first embeddings stream ended unexpectedly"};

        if (!second)
            throw word_embeddings_exception{
                "second embeddings stream ended unexpectedly"};

        progress(tid);
        auto vec = vector(tid);
        std::generate(vec.begin(), vec.end(), [&]()
                      {
                          return (io::packed::read<double>(first)
                                  + io::packed::read<double>(second));
                      });
        auto len = math::operators::l2norm(vec);
        std::transform(vec.begin(), vec.end(), vec.begin(), [=](double weight)
                {
                    return weight / len;
                });
    }
}

void word_embeddings::load_vocab(std::istream& vocab)
{
    printing::progress progress{" > Loading vocab: ", id_to_term_.size()};
    for (std::size_t tid = 0; tid < id_to_term_.size(); ++tid)
    {
        if (!vocab)
            throw word_embeddings_exception{"vocab stream ended unexpectedly"};

        progress(tid);
        io::packed::read(vocab, id_to_term_[tid]);
        term_to_id_[id_to_term_[tid]] = tid;

        // discard the count
        io::packed::read<std::size_t>(vocab);
    }
}

util::array_view<double> word_embeddings::vector(std::size_t tid)
{
    return {embeddings_.data() + tid * vector_size_, vector_size_};
}

util::array_view<const double> word_embeddings::vector(std::size_t tid) const
{
    return {embeddings_.data() + tid * vector_size_, vector_size_};
}

embedding word_embeddings::at(util::string_view term) const
{
    std::size_t tid;
    auto v_it = term_to_id_.find(term);
    if (v_it == term_to_id_.end())
    {
        tid = id_to_term_.size();
    }
    else
    {
        tid = v_it->value();
    }
    return {tid, vector(tid)};
}

util::string_view word_embeddings::term(std::size_t tid) const
{
    if (tid >= id_to_term_.size())
        return "<unk>";
    return id_to_term_[tid];
}

std::vector<scored_embedding>
word_embeddings::top_k(util::array_view<const double> query,
                       std::size_t k) const
{
    auto comp = [](const scored_embedding& a, const scored_embedding& b)
    {
        return a.score > b.score;
    };
    util::fixed_heap<scored_embedding, decltype(comp)> results{k, comp};

    // +1 for <unk>
    for (std::size_t tid = 0; tid < id_to_term_.size() + 1; ++tid)
    {
        auto vec = vector(tid);
        auto score
            = std::inner_product(query.begin(), query.end(), vec.begin(), 0.0);

        embedding e{tid, vec};
        results.push({e, score});
    }

    return results.extract_top();
}

word_embeddings load_embeddings(const cpptoml::table& config)
{
    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw word_embeddings_exception{
            "missing prefix key in configuration file"};

    std::ifstream vocab{*prefix + "/vocab.bin", std::ios::binary};
    if (!vocab)
        throw word_embeddings_exception{"missing vocabulary file in: "
                                        + *prefix};

    std::ifstream target{*prefix + "/embeddings.target.bin", std::ios::binary};
    std::ifstream context{*prefix + "/embeddings.context.bin",
                          std::ios::binary};

    auto mode = config.get_as<std::string>("mode").value_or("average");
    if (mode == "average")
    {
        if (!target)
            throw word_embeddings_exception{"missing target vectors in: "
                                            + *prefix};
        if (!context)
            throw word_embeddings_exception{"missing context vectors in: "
                                            + *prefix};

        return {vocab, target, context};
    }
    else if (mode == "target")
    {
        if (!target)
            throw word_embeddings_exception{"missing target vectors in: "
                                            + *prefix};

        return {vocab, target};
    }
    else if (mode == "context")
    {
        if (!context)
            throw word_embeddings_exception{"missing context vectors in: "
                                            + *prefix};

        return {vocab, context};
    }
    else
    {
        throw word_embeddings_exception{"invalid mode key in configuration"};
    }
}
}
}
