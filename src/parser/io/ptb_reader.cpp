/**
 * @file ptb_reader.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include <fstream>
#include "parser/io/ptb_reader.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"
#include "utf/utf.h"
#include "util/shim.h"

namespace meta
{
namespace parser
{
namespace io
{

namespace
{

void read_whitespace(std::ifstream& file)
{
    while (file && std::isspace(file.get()))
    {
    }
    file.unget();
}

void read_lparen(std::ifstream& file)
{
    if (!file || file.peek() != '(')
        throw std::runtime_error{
            "invalid tree format (expected '(' instead of '"
            + std::to_string(file.peek()) + "')"};

    file.get();
}

void read_rparen(std::ifstream& file)
{
    if (!file || file.peek() != ')')
        throw std::runtime_error{
            "invalid tree format (expected ')' instead of '"
            + std::to_string(file.peek()) + "')"};
    file.get();
}

std::string read_word(std::ifstream& file)
{
    std::string word;
    while (file && file.peek() != '(' && file.peek() != ')'
           && !std::isspace(file.peek()))
        word += file.get();

    if (word.length() == 0)
        throw std::runtime_error{"invalid tree format reading text"};

    return word;
}

class_label read_label(std::ifstream& file)
{
    read_whitespace(file);
    return class_label{read_word(file)};
}

std::unique_ptr<node> read_subtree(std::ifstream& file)
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

std::unique_ptr<node> read_tree(std::ifstream& file)
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

std::vector<parse_tree> extract_trees(const std::string& filename)
{
    std::vector<parse_tree> results;

    std::ifstream file{filename};
    read_whitespace(file);

    while (file)
    {
        results.emplace_back(read_tree(file));
        read_whitespace(file);
    }

    return results;
}
}
}
}
