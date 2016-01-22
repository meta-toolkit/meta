/**
 * @file diff_analyzer.cpp
 * @author Sean Massung
 */

#include <string>
#include <vector>

#include "meta/corpus/document.h"
#include "meta/lm/analyzers/diff_analyzer.h"
#include "meta/analyzers/token_stream.h"

namespace meta
{
namespace analyzers
{

const util::string_view diff_analyzer::id = "diff";

diff_analyzer::diff_analyzer(const cpptoml::table& config,
                             std::unique_ptr<token_stream> stream)
    : stream_{std::move(stream)}, diff_{std::make_shared<lm::diff>(config)}
{
    // nothing
}

diff_analyzer::diff_analyzer(const diff_analyzer& other)
    : stream_{other.stream_->clone()}, diff_{other.diff_}
{
    // nothing
}

void diff_analyzer::tokenize(const corpus::document& doc, featurizer& counts)
{
    // first, get tokens
    stream_->set_content(get_content(doc));
    std::vector<std::string> sentences;
    std::string buffer{""};

    while (*stream_)
    {
        auto next = stream_->next();
        if (next == "</s>")
        {
            // sentence constructor adds <s> and </s> itself
            sentences.emplace_back(std::move(buffer));
            continue;
        }
        else if (next != "<s>")
            buffer += next + " ";
    }

    for (auto& s : sentences)
    {
        try
        {
            lm::sentence sent{s};
            auto candidates = diff_->candidates(sent, true);
            auto edits = candidates[0].first.operations();
            if (edits.empty())
                counts("unmodified", 1ul);
            else
            {
                for (auto& e : edits)
                    counts(e, 1ul);
            }
        }
        catch (lm::sentence_exception& ex)
        {
            counts("no-candidates", 1ul);
        }
    }
}

template <>
std::unique_ptr<analyzer>
make_analyzer<diff_analyzer>(const cpptoml::table& global,
                             const cpptoml::table& config)
{
    auto filts = load_filters(global, config);
    return make_unique<diff_analyzer>(global, std::move(filts));
}
}
}
