/**
 * @file interactive_embeddings.cpp
 * @author Chase Geigle
 *
 * This tool is an interactive demo over learned word embeddings. Each
 * query will be interpreted down to a unit-length vector and the top 100
 * closest embeddings to that query will be printed along with their score.
 */

#include <cctype>
#include <iostream>
#include <string>
#include <stdexcept>

#include "cpptoml.h"
#include "meta/embeddings/word_embeddings.h"
#include "meta/logging/logger.h"
#include "meta/math/vector.h"

using namespace meta;

class parse_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

void parse_whitespace(util::string_view& query)
{
    while (!query.empty() && std::isspace(query.front()))
        query = query.substr(1);
}

util::array_view<const double>
parse_word(util::string_view& query, const embeddings::word_embeddings& glove)
{
    parse_whitespace(query);
    auto word = query;
    while (!query.empty() && (!std::isspace(query.front())
                              && query.front() != '+' && query.front() != '-'))
    {
        query = query.substr(1);
    }
    word = word.substr(0, word.length() - query.length());
    if (word.empty())
        throw parse_exception{"invalid expression"};
    parse_whitespace(query);
    return glove.at(word).v;
}

std::vector<double> parse_expression(util::string_view& query,
                                     const embeddings::word_embeddings& glove)
{
    if (query.empty())
        throw parse_exception{"invalid expression"};

    using namespace meta::math::operators;
    auto pos = query.find_last_of("+-");
    if (pos == util::string_view::npos)
    {
        auto vec = parse_word(query, glove);
        return {vec.begin(), vec.end()};
    }

    auto left_expr = query.substr(0, pos);
    auto left = parse_expression(left_expr, glove);
    query = query.substr(pos);
    if (query.empty())
        throw parse_exception{"invalid expression"};
    auto op = query.front();
    query = query.substr(1);
    auto right = parse_word(query, glove);

    switch (op)
    {
        case '+':
            return left + right;
        case '-':
            return left - right;
        default:
            throw parse_exception{"invalid expression"};
    }
}

std::vector<double> parse_query(util::string_view query,
                                const embeddings::word_embeddings& glove)
{
    using namespace meta::math::operators;
    auto vec = parse_expression(query, glove);
    auto len = l2norm(vec);
    return std::move(vec) / len;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto embed_cfg = config->get_table("embeddings");
    if (!embed_cfg)
    {
        std::cerr << "Missing [embeddings] configuration in " << argv[1]
                  << std::endl;
        return 1;
    }

    auto glove = embeddings::load_embeddings(*embed_cfg);

    std::cout << "Enter a query and press enter (empty to quit).\n> "
              << std::flush;

    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line.empty())
            break;

        try
        {
            auto query = parse_query(line, glove);
            for (const auto& se : glove.top_k(query, 10))
            {
                auto term = glove.term(se.e.tid);
                std::cout << term << " (" << se.score << ")\n";
            }
            std::cout << std::endl;
        }
        catch (const parse_exception& ex)
        {
            std::cout << "error: " << ex.what() << std::endl;
        }

        std::cout << "> " << std::flush;
    }

    return 0;
}
