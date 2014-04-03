/**
 * @file libsvm_parser.cpp
 * @author Sean Massung
 */

#include <sstream>

#include "io/libsvm_parser.h"

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

counts_t counts(const std::string& text, bool contains_label /* = true */)
{
    std::stringstream stream{text};
    std::string token;

    if (contains_label)
    {
        if (!(stream >> token)) // ignore class label, but check that it's there
            throw libsvm_parser_exception{
                "incorrectly formatted libsvm data: " + text};
    }

    std::vector<std::pair<term_id, double>> counts;
    term_id term;
    double count;
    while (stream >> token)
    {
        size_t colon = token.find_first_of(':');
        if (colon == std::string::npos || colon == 0 || colon == token.size()
                                                                 - 1)
            throw libsvm_parser_exception{"incorrectly formatted libsvm data: "
                                          + text};

        std::istringstream term_stream{token.substr(0, colon)};
        term_stream >> term;
        std::istringstream double_stream{token.substr(colon + 1)};
        double_stream >> count;

        // make sure double conversion worked and used the entire string
        if (double_stream.fail() || !double_stream.eof())
            throw libsvm_parser_exception{"incorrectly formatted libsvm data: "
                                          + text};

        if (term == 0)
            throw libsvm_parser_exception{"term id was 0 from libsvm format"};

        // liblinear has term_ids start at 1 instead of 0 like MeTA and libsvm
        term_id minus_term{static_cast<uint64_t>(term) - 1};
        counts.emplace_back(minus_term, count);
    }

    return counts;
}
}
}
}
