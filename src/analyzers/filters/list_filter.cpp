/**
 * @file list_filter.cpp
 * @author Chase Geigle
 */

#include <fstream>
#include "meta/analyzers/filters/list_filter.h"
#include "cpptoml.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const util::string_view list_filter::id = "list";

list_filter::list_filter(std::unique_ptr<token_stream> source,
                         const std::string& filename, type method)
    : source_{std::move(source)}, method_{method}
{
    std::ifstream file{filename};
    if (!file)
        throw token_stream_exception{"invalid file for list filter"};

    std::string line;
    while (std::getline(file, line))
        list_.emplace(std::move(line));

    next_token();
}

list_filter::list_filter(const list_filter& other)
    : source_{other.source_->clone()},
      token_{other.token_},
      list_{other.list_},
      method_{other.method_}
{
    // nothing
}

void list_filter::set_content(std::string&& content)
{
    token_ = util::nullopt;
    source_->set_content(std::move(content));
    next_token();
}

std::string list_filter::next()
{
    auto tok = std::move(*token_);
    next_token();
    return tok;
}

list_filter::operator bool() const
{
    return token_ || *source_;
}

void list_filter::next_token()
{
    if (!*source_)
    {
        token_ = util::nullopt;
        return;
    }

    while (*source_)
    {
        auto tok = source_->next();
        auto found = list_.find(tok) != list_.end();
        switch (method_)
        {
            case type::ACCEPT:
                if (found)
                {
                    token_ = std::move(tok);
                    return;
                }
                break;
            case type::REJECT:
                if (!found)
                {
                    token_ = std::move(tok);
                    return;
                }
                break;
            default:
                throw token_stream_exception{"invalid method"};
        }
    }
    token_ = util::nullopt;
}

template <>
std::unique_ptr<token_stream>
    make_filter<list_filter>(std::unique_ptr<token_stream> src,
                             const cpptoml::table& config)
{
    auto method = config.get_as<std::string>("method");
    auto file = config.get_as<std::string>("file");
    if (!file)
        throw token_stream_exception{"file required for list_filter config"};

    list_filter::type type = list_filter::type::REJECT;
    if (method)
    {
        if (*method == "accept")
            type = list_filter::type::ACCEPT;
        else if (*method != "reject")
            throw token_stream_exception{"invalid method for list_filter"};
    }

    return make_unique<list_filter>(std::move(src), *file, type);
}
}
}
}
