/**
 * @file mph_language_model.cpp
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <cassert>

#include "meta/io/filesystem.h"
#include "meta/lm/mph_language_model.h"
#include "meta/lm/ngram_map.h"
#include "meta/lm/read_arpa.h"
#include "meta/util/algorithm.h"
#include "meta/util/shim.h"
#include "meta/util/time.h"

namespace meta
{
namespace lm
{

namespace
{
class ngram_handler
{
  public:
    using unigram_builder_type = ngram_map_builder<std::string>;
    using middle_builder_type = ngram_map_builder<std::vector<term_id>>;
    using last_builder_type = ngram_map_builder<std::vector<term_id>, float>;

    ngram_handler(const std::string& prefix) : prefix_{prefix}
    {
        // nothing
    }

    void count(uint64_t ngram_count)
    {
        counts_.emplace_back(ngram_count);
        LOG(info) << counts_.size() << "-gram count: " << ngram_count << ENDLG;
        if (counts_.size() == 1)
        {
            unigram_builder_type::options_type options;
            options.prefix = prefix_ + "/0/";
            options.num_keys = ngram_count;

            filesystem::make_directory(options.prefix);

            unigram_builder_ = make_unique<unigram_builder_type>(options);
            progress_ = make_unique<printing::progress>(" > Reading 1-grams: ",
                                                        counts_.front());
        }
    }

    void operator()(uint64_t order, const std::string& ngram, float prob,
                    float backoff)
    {
        if (order > order_)
        {
            finish_order();
            order_ = order;
        }

        (*progress_)(observed_++);
        if (observed_ > counts_[order])
            throw std::runtime_error{"too many " + std::to_string(order + 1)
                                     + "-grams"};

        if (order_ == 0)
        {
            assert(unigram_builder_);
            prob_backoff<> value;
            value.prob = prob;
            value.backoff = backoff;
            (*unigram_builder_)(ngram, value);
        }
        else
        {
            assert(middle_builder_ || last_builder_);
            assert(unigrams_);

            std::vector<term_id> ids;
            ids.reserve(order);

            util::for_each_token(
                ngram.begin(), ngram.end(), " ",
                [&](std::string::const_iterator first,
                    std::string::const_iterator last) {
                    if (first != last)
                    {
                        auto unigram = util::make_string_view(first, last);
                        auto id = unigrams_->index(unigram);
                        if (!id)
                            throw std::runtime_error{
                                "ngram contains unknown unigram "
                                + unigram.to_string()};
                        ids.push_back(term_id{*id});
                    }
                });

            assert(ids.size() == order + 1);

            prob_backoff<> value;
            value.prob = prob;
            value.backoff = backoff;

            if (order_ != counts_.size() - 1)
                (*middle_builder_)(ids, value);
            else
                (*last_builder_)(ids, prob);
        }
    }

    uint64_t order() const
    {
        return order_;
    }

    void finish_order()
    {
        progress_ = nullptr;
        LOG(info) << "Finalizing " << order_ + 1 << "-grams (" << observed_
                  << ")" << ENDLG;
        observed_ = 0;
        // unigrams
        if (order_ == 0)
        {
            assert(unigram_builder_);
            unigram_builder_->write();
            unigram_builder_ = nullptr;

            // now that the unigrams are written, we're going to load their
            // ngram_map to use as a vocabulary lookup
            unigrams_
                = make_unique<ngram_map<util::string_view>>(prefix_ + "/0");
            LOG(info) << "Loaded unigram map" << ENDLG;

            middle_builder_type::options_type options;
            options.prefix = prefix_ + "/" + std::to_string(order_ + 1);
            options.num_keys = counts_[order_ + 1];

            filesystem::make_directory(options.prefix);

            middle_builder_ = make_unique<middle_builder_type>(options);
            progress_ = make_unique<printing::progress>(" > Reading 2-grams: ",
                                                        options.num_keys);
        }
        // middle
        else if (order_ < counts_.size() - 1)
        {
            assert(middle_builder_);
            middle_builder_->write();

            // set up the next middle builder
            if (order_ + 1 < counts_.size() - 1)
            {
                middle_builder_type::options_type options;
                options.prefix = prefix_ + "/" + std::to_string(order_ + 1);
                options.num_keys = counts_[order_ + 1];

                filesystem::make_directory(options.prefix);

                middle_builder_ = make_unique<middle_builder_type>(options);
                progress_ = make_unique<printing::progress>(
                    " > Reading " + std::to_string(order_ + 2) + "-grams: ",
                    options.num_keys);
            }
            // here come the final ngrams
            else
            {
                middle_builder_ = nullptr;

                last_builder_type::options_type options;
                options.prefix = prefix_ + "/" + std::to_string(order_ + 1);
                options.num_keys = counts_[order_ + 1];

                filesystem::make_directory(options.prefix);

                last_builder_ = make_unique<last_builder_type>(options);
                progress_ = make_unique<printing::progress>(
                    " > Reading " + std::to_string(order_ + 2) + "-grams: ",
                    options.num_keys);
            }
        }
        // last
        else
        {
            assert(last_builder_);
            last_builder_->write();
            last_builder_ = nullptr;
        }
    }

  private:
    std::string prefix_;
    uint64_t order_ = 0;
    uint64_t observed_ = 0;
    std::vector<uint64_t> counts_;
    std::unique_ptr<printing::progress> progress_;
    std::unique_ptr<unigram_builder_type> unigram_builder_;
    std::unique_ptr<ngram_map<util::string_view>> unigrams_;
    std::unique_ptr<middle_builder_type> middle_builder_;
    std::unique_ptr<last_builder_type> last_builder_;
};

uint64_t build_from_arpa(const std::string& arpa_file,
                         const std::string& prefix)
{
    filesystem::remove_all(prefix);
    filesystem::make_directory(prefix);
    ngram_handler handler{prefix};

    std::ifstream arpa{arpa_file};
    read_arpa(arpa, [&](uint64_t /* order */,
                        uint64_t count) { handler.count(count); },
              handler);

    // finish off the n-grams
    handler.finish_order();

    return handler.order();
}
}

struct mph_language_model::impl
{
    using unigram_map_type = ngram_map<util::string_view>;
    using middle_map_type = ngram_map<std::vector<term_id>>;
    using last_map_type = ngram_map<std::vector<term_id>, float>;

    impl(const std::string& prefix, uint64_t ord)
        : order{ord},
          unigrams{prefix + "/0"},
          last{prefix + "/" + std::to_string(order)}
    {
        for (uint64_t i = 1; i < order; ++i)
        {
            auto mid = make_unique<middle_map_type>(prefix + "/"
                                                    + std::to_string(i));
            middle_vec.push_back(std::move(mid));
        }

        unk = *unigrams.index_and_value("<unk>");
    }

    const middle_map_type& middle(uint64_t len) const
    {
        return *middle_vec[len - 2];
    }

    uint64_t order;
    hashing::indexed_value<prob_backoff<>> unk;
    unigram_map_type unigrams;
    std::vector<std::unique_ptr<middle_map_type>> middle_vec;
    last_map_type last;
};

mph_language_model::mph_language_model(const cpptoml::table& config)
{
    auto table = config.get_table("mph-language-model");
    auto arpa_file = table->get_as<std::string>("arpa-file");
    auto prefix = table->get_as<std::string>("binary-file-prefix");

    util::optional<uint64_t> order;
    if (!filesystem::file_exists(*prefix + "/0/values.bin"))
    {
        LOG(info) << "Building language model from .arpa file: " << *arpa_file
                  << ENDLG;

        auto time = common::time(
            [&]() { order = build_from_arpa(*arpa_file, *prefix); });
        LOG(info) << "Done. (" << time.count() << "ms)" << ENDLG;
    }
    LOG(info) << "Loading language model from binary files in: " << *prefix
              << ENDLG;

    if (!order)
    {
        for (uint64_t ord = 0; filesystem::file_exists(
                 *prefix + "/" + std::to_string(ord) + "/values.bin");
             ++ord)
            order = ord;
    }

    impl_ = make_unique<impl>(*prefix, *order);
}

term_id mph_language_model::index(util::string_view token) const
{
    return term_id{impl_->unigrams.index(token).value_or(impl_->unk.idx)};
}

term_id mph_language_model::unk() const
{
    return term_id{impl_->unk.idx};
}

float mph_language_model::score(const lm_state& in_state,
                                util::string_view token,
                                lm_state& out_state) const
{
    auto iav = impl_->unigrams.index_and_value(token).value_or(impl_->unk);
    return score(in_state, term_id{iav.idx}, iav.value, out_state);
}

float mph_language_model::score(const lm_state& in_state, term_id token,
                                lm_state& out_state) const
{
    return score(in_state, token, impl_->unigrams[token], out_state);
}

float mph_language_model::score(const lm_state& in_state, term_id token,
                                prob_backoff<> pb, lm_state& out_state) const
{
    out_state = in_state;
    out_state.previous.push_back(token);

    // (1) Find the longest matching ngram
    if (out_state.previous.size() == impl_->order + 1)
    {
        if (auto full = impl_->last.at(out_state.previous))
        {
            out_state.shrink();
            return *full;
        }
        out_state.shrink();
    }

    float res = 0;
    while (out_state.previous.size() > 1)
    {
        const auto& table = impl_->middle(out_state.previous.size());
        if (auto mid = table.at(out_state.previous))
        {
            res = mid->prob;
            break;
        }
        out_state.shrink();
    }

    if (out_state.previous.size() == 1)
        res = pb.prob;

    if (out_state.previous.size() > in_state.previous.size())
        return res;

    // (2) Apply backoff penalties if needed
    auto backoff = in_state;
    for (uint64_t i = 0;
         i < in_state.previous.size() - out_state.previous.size() + 1; ++i)
    {
        if (backoff.previous.size() == 1)
        {
            res += impl_->unigrams[backoff.previous.front()].backoff;
        }
        else
        {
            const auto& table = impl_->middle(backoff.previous.size());
            res += table.at(backoff.previous)->backoff;
            backoff.shrink();
        }
    }

    return res;
}

mph_language_model::~mph_language_model() = default;
}
}
