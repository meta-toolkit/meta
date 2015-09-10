/**
 * @file diff_analyzer.cpp
 * @author Sean Massung
 */

#include <string>
#include <vector>

#include "corpus/document.h"
#include "lm/analyzers/diff_analyzer.h"
#include "analyzers/token_stream.h"

namespace meta
{
namespace analyzers
{

const std::string diff_analyzer::id = "diff";

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

void diff_analyzer::tokenize(corpus::document& doc)
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

    for(auto& s: sentences)
    {
        try
        {
            lm::sentence sent{s};
            auto candidates = diff_->candidates(sent, true);
            auto edits = candidates[0].first.operations();
            if (edits.empty())
                doc.increment("unmodified", 1);
            else
            {
                for (auto& e : edits)
                    doc.increment(e, 1);
            }
        }
        catch (lm::sentence_exception& ex)
        {
            doc.increment("no-candidates", 1);
        }

    }
}

template <>
std::unique_ptr<analyzer>
    make_analyzer<diff_analyzer>(const cpptoml::table& global,
                                 const cpptoml::table& config)
{
    auto filts = analyzer::load_filters(global, config);
    return make_unique<diff_analyzer>(global, std::move(filts));
}
}
}
