/**
 * @file embedding_analyzer.cpp
 * @author Sean Massung
 */

#include "meta/analyzers/token_stream.h"
#include "meta/corpus/document.h"
#include "meta/embeddings/analyzers/embedding_analyzer.h"
#include "meta/math/vector.h"

namespace meta
{
namespace analyzers
{

const util::string_view embedding_analyzer::id = "embedding";

embedding_analyzer::embedding_analyzer(const cpptoml::table& config,
                                       std::unique_ptr<token_stream> stream)
    : stream_{std::move(stream)},
      embeddings_{std::make_shared<embeddings::word_embeddings>(
          embeddings::load_embeddings(config))},
      prefix_{*config.get_as<std::string>("prefix")},
      features_(embeddings_->vector_size(), 0.0)
{
    // nothing
}

embedding_analyzer::embedding_analyzer(const embedding_analyzer& other)
    : stream_{other.stream_->clone()},
      embeddings_{other.embeddings_},
      prefix_{other.prefix_},
      features_{other.features_}
{
    // nothing
}

void embedding_analyzer::tokenize(const corpus::document& doc,
                                  featurizer& counts)
{
    using namespace math::operators;
    stream_->set_content(get_content(doc));
    features_.assign(embeddings_->vector_size(), 0.0);
    uint64_t num_seen = 0;
    while (*stream_)
    {
        auto token = stream_->next();
        features_ = std::move(features_) + embeddings_->at(token).v;
        ++num_seen;
    }

    // average each feature and record it
    uint64_t cur_dim = 0;
    for (const auto& val : features_)
        counts(prefix_ + std::to_string(cur_dim++), val / num_seen);
}

template <>
std::unique_ptr<analyzer>
make_analyzer<embedding_analyzer>(const cpptoml::table& global,
                                  const cpptoml::table& config)
{
    auto filts = load_filters(global, config);
    return make_unique<embedding_analyzer>(config, std::move(filts));
}
}

namespace embeddings
{
void register_analyzers()
{
    using namespace analyzers;
    register_analyzer<embedding_analyzer>();
}
}
}
