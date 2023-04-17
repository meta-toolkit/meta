/**
 * @file libsvm_parser.cpp
 * @author Sean Massung
 */

#include <cstdlib>

#include "meta/io/libsvm_parser.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace io
{

namespace libsvm_parser
{

class_label label(const std::string& text)
{
    size_t space = text.find_first_of(' ');
    if (space == std::string::npos || space == 0 || space == text.size() - 1)
        throw libsvm_parser_exception{"incorrectly formatted libsvm data: "
                                      + text};

    return class_label{text.substr(0, space)};
}

void throw_exception(const std::string& text)
{
    throw libsvm_parser_exception{"incorrectly formatted libsvm data: " + text};
}

counts_t counts(const std::string& text, bool contains_label /* = true */)
{
    util::string_view sv{text};

    if (contains_label)
    {
        auto pos = sv.find_first_of(" \t");
        if (pos == std::string::npos || pos == 0)
            throw_exception(text);
        sv = sv.substr(pos);
    }

    auto consume_whitespace = [&]() {
        auto pos = sv.find_first_not_of(" \t");
        if (pos != sv.npos)
            sv = sv.substr(pos);
        else
            sv = util::string_view{}; // .clear() doesn't exist for GCC...
    };

    consume_whitespace();

    std::vector<std::pair<term_id, double>> counts;
    while (!sv.empty())
    {
        auto whitespace = sv.find_first_of(" \t");
        auto token = sv.substr(0, whitespace);

        if (token.empty())
            throw_exception("empty token");

        auto colon = token.find_first_of(":");
        if (colon == std::string::npos || colon == 0
            || colon == token.size() - 1)
        {
            throw_exception("no colon in token: " + util::to_string(token));
        }

        char* end = nullptr;
        auto term = std::strtoul(token.data(), nullptr, 0);
        double count = std::strtod(token.substr(colon + 1).data(), &end);

        if (end != token.data() + token.size())
        {
            throw_exception("full token not consumed: "
                            + util::to_string(token));
        }

        if (term == 0)
            throw libsvm_parser_exception{"term id was 0 from libsvm format"};

        // liblinear has term_ids start at 1 instead of 0 like MeTA and libsvm
        term_id minus_term{term - 1};
        counts.emplace_back(minus_term, count);

        if (whitespace == std::string::npos)
            break;

        sv = sv.substr(whitespace);
        consume_whitespace();
    }

    return counts;
}
} // namespace libsvm_parser
} // namespace io
} // namespace meta
