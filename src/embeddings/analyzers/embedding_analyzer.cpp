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
    : stream_{std::move(stream)}

{
    auto grp = config.get_table("embeddings");
    if (!grp)
        throw std::runtime_error{"[embeddings] section needed in config"};

    embeddings_ = std::make_shared<embeddings::word_embeddings>(
        embeddings::load_embeddings(*grp));
}

embedding_analyzer::embedding_analyzer(const embedding_analyzer& other)
    : stream_{other.stream_->clone()}, embeddings_{other.embeddings_}
{
    // nothing
}

void embedding_analyzer::tokenize(const corpus::document& doc,
                                  featurizer& counts)
{
    using namespace math::operators;
    stream_->set_content(get_content(doc));
    std::vector<double> features(embeddings_->vector_size(), 0.0);
    uint64_t num_seen = 0;
    while (*stream_)
    {
        auto token = stream_->next();
        features = features + embeddings_->at(token).v;
        ++num_seen;
    }

    // average each feature and record it
    uint64_t cur_dim = 0;
    for (const auto& val : features)
        counts(std::to_string(cur_dim++), val / num_seen);
}

template <>
std::unique_ptr<analyzer>
make_analyzer<embedding_analyzer>(const cpptoml::table& global,
                                  const cpptoml::table& config)
{
    auto filts = load_filters(global, config);
    return make_unique<embedding_analyzer>(global, std::move(filts));
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
