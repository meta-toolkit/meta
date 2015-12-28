/**
 * @file ptb_reader.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include <fstream>
#include "meta/parser/io/ptb_reader.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/util/shim.h"

namespace meta
{
namespace parser
{
namespace io
{

namespace
{

void read_whitespace(std::istream& file)
{
    while (file && std::isspace(file.get()))
    {
    }
    file.unget();
}

void read_lparen(std::istream& file)
{
    if (!file || file.peek() != '(')
        throw std::runtime_error{
            "invalid tree format (expected '(' instead of '"
            + std::to_string(file.peek()) + "')"};

    file.get();
}

void read_rparen(std::istream& file)
{
    if (!file || file.peek() != ')')
        throw std::runtime_error{
            "invalid tree format (expected ')' instead of '"
            + std::to_string(file.peek()) + "')"};
    file.get();
}

std::string read_word(std::istream& file)
{
    std::string word;
    while (file && file.peek() != '(' && file.peek() != ')'
           && !std::isspace(file.peek()))
        word += static_cast<char>(file.get());

    if (word.length() == 0)
        throw std::runtime_error{"invalid tree format reading text"};

    return word;
}

class_label read_label(std::istream& file)
{
    read_whitespace(file);
    return class_label{read_word(file)};
}

std::unique_ptr<node> read_subtree(std::istream& file)
{
    read_whitespace(file);
    read_lparen(file);

    auto label = read_label(file);
    read_whitespace(file);

    if (file.peek() != '(')
    {
        std::string word = read_word(file);
        auto n = make_unique<leaf_node>(std::move(label), std::move(word));
        read_whitespace(file);
        read_rparen(file);
        return std::move(n);
    }
    else
    {
        auto n = make_unique<internal_node>(label);
        while (file && file.peek() != ')')
        {
            n->add_child(read_subtree(file));
            read_whitespace(file);
        }
        read_rparen(file);
        return std::move(n);
    }
}

std::unique_ptr<node> read_tree(std::istream& file)
{
    read_lparen(file);

    auto node = make_unique<internal_node>(class_label{"ROOT"});
    while (file && file.peek() != ')')
    {
        node->add_child(read_subtree(file));
        read_whitespace(file);
    }
    read_rparen(file);

    return std::move(node);
}
}

std::vector<parse_tree> extract_trees(std::istream& stream)
{
    std::vector<parse_tree> results;

    read_whitespace(stream);

    while (stream)
    {
        results.emplace_back(read_tree(stream));
        read_whitespace(stream);
    }

    return results;
}

std::vector<parse_tree> extract_trees(const std::string& filename)
{
    std::ifstream file{filename};
    return extract_trees(file);
}

}
}
}
