/**
 * @file ngram_word_analyzer.cpp
 * @author Sean Massung
 */

#include <string>
#include <vector>

#include "cpptoml.h"
#include "meta/corpus/document.h"
#include "meta/analyzers/ngram/ngram_word_analyzer.h"
#include "meta/analyzers/token_stream.h"

namespace meta
{
namespace analyzers
{

template <class T>
const util::string_view ngram_word_analyzer<T>::id = "ngram-word";

template <class T>
ngram_word_analyzer<T>::ngram_word_analyzer(
    uint16_t n, std::unique_ptr<token_stream> stream)
    : base{n}, stream_{std::move(stream)}
{
    // nothing
}

template <class T>
ngram_word_analyzer<T>::ngram_word_analyzer(const ngram_word_analyzer& other)
    : base{other.n_value()}, stream_{other.stream_->clone()}
{
    // nothing
}

template <class T>
void ngram_word_analyzer<T>::tokenize(const corpus::document& doc,
                                      feature_map& counts)
{
    stream_->set_content(get_content(doc));
    std::deque<std::string> tokens;
    while (*stream_)
    {
        tokens.emplace_back(stream_->next());
        if (tokens.size() == this->n_value())
        {
            auto combined = std::move(tokens.front());
            tokens.pop_front();
            for (const auto& token : tokens)
                combined += "_" + token;

            counts[combined] += 1;
        }
    }
}

template <class T>
std::unique_ptr<analyzer<T>> analyzer_traits<ngram_word_analyzer<T>>::create(
    const cpptoml::table& global, const cpptoml::table& config)
{
    auto n_val = config.get_as<int64_t>("ngram");
    if (!n_val)
        throw analyzer_exception{
            "ngram size needed for ngram word analyzer in config file"};

    auto filts = load_filters(global, config);
    return make_unique<ngram_word_analyzer<T>>(*n_val, std::move(filts));
}

template class ngram_word_analyzer<uint64_t>;
template class ngram_word_analyzer<double>;
template struct analyzer_traits<ngram_word_analyzer<uint64_t>>;
template struct analyzer_traits<ngram_word_analyzer<double>>;
}
}
