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

template <class T>
const util::string_view diff_analyzer<T>::id = "diff";

template <class T>
diff_analyzer<T>::diff_analyzer(const cpptoml::table& config,
                                std::unique_ptr<token_stream> stream)
    : stream_{std::move(stream)}, diff_{std::make_shared<lm::diff>(config)}
{
    // nothing
}

template <class T>
diff_analyzer<T>::diff_analyzer(const diff_analyzer& other)
    : stream_{other.stream_->clone()}, diff_{other.diff_}
{
    // nothing
}

template <class T>
void diff_analyzer<T>::tokenize(const corpus::document& doc,
                                feature_map& counts)
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
                counts["unmodified"] += 1;
            else
            {
                for (auto& e : edits)
                    counts[e] += 1;
            }
        }
        catch (lm::sentence_exception& ex)
        {
            counts["no-candidates"] += 1;
        }
    }
}

template <class T>
std::unique_ptr<analyzer<T>>
    analyzer_traits<diff_analyzer<T>>::create(const cpptoml::table& global,
                                              const cpptoml::table& config)
{
    auto filts = load_filters(global, config);
    return make_unique<diff_analyzer<T>>(global, std::move(filts));
}

template class diff_analyzer<uint64_t>;
template class diff_analyzer<double>;
template struct analyzer_traits<diff_analyzer<uint64_t>>;
template struct analyzer_traits<diff_analyzer<double>>;
}
}
