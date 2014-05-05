/**
 * @file parse_tree.cpp
 */

#include <fstream>

#include "corpus/document.h"
#include "analyzers/tree/parse_tree.h"

namespace meta
{
namespace analyzers
{

parse_tree::parse_tree(const std::string& tags)
{
    syntactic_category_ = root_category(tags);
    std::vector<std::string> trans{transitions(tags)};
    for (auto& transition : trans)
        children_.push_back(parse_tree(transition));
}

std::vector<std::string> parse_tree::transitions(std::string tags) const
{
    // make sure there are actually transitions
    if (tags == "" || tags.substr(1, tags.size() - 1).find_first_of("(")
                      == std::string::npos)
        return std::vector<std::string>{};

    // get rid of first transition and its closing paren
    uint64_t index = 1;
    while (tags[index] != '(')
        ++index;
    tags = tags.substr(index, tags.size() - index - 1);

    // find groups of same-level parens; start on the first paren
    index = 0;
    int paren_depth = 0;
    std::string current{""};
    std::vector<std::string> transitions;
    while (index < tags.size())
    {
        current += tags[index];
        if (tags[index] == ')')
            --paren_depth;
        else if (tags[index] == '(')
            ++paren_depth;

        if (paren_depth == 0)
        {
            transitions.push_back(current);
            current = "";
        }
        ++index;
    }

    return transitions;
}

std::string parse_tree::root_category(const std::string& tags) const
{
    uint64_t index = 1;
    std::string cat{""};
    while (tags[index] != ')' && tags[index] != '(')
        cat += tags[index++];
    return cat;
}

std::string parse_tree::get_category() const
{
    return syntactic_category_;
}

std::vector<parse_tree> parse_tree::children() const
{
    return children_;
}

uint64_t parse_tree::num_children() const
{
    return children_.size();
}

std::string parse_tree::get_string() const
{
    std::string ret{"(" + syntactic_category_};
    for (auto& child : children_)
        ret += child.get_string();
    ret += ")";
    return ret;
}

std::string parse_tree::skeleton() const
{
    std::string ret{"("};
    for (auto& child : children_)
        ret += child.skeleton();
    ret += ")";
    return ret;
}

std::string parse_tree::pretty_print(const parse_tree& tree)
{
    std::stringstream output;
    pretty_print(tree, 0, output);
    return output.str();
}

void parse_tree::pretty_print(const parse_tree& tree, uint64_t depth,
                              std::stringstream& output)
{
    std::string padding(depth, ' ');
    output << padding << "(" << std::endl << padding << "  "
           << tree.syntactic_category_ << std::endl;
    for (auto& child : tree.children_)
        pretty_print(child, depth + 2, output);
    output << padding << ")" << std::endl;
}

std::string parse_tree::get_children_string() const
{
    std::string ret{""};
    for (auto& child : children_)
        ret += "(" + child.syntactic_category_ + ")";
    return ret;
}

std::vector<parse_tree> parse_tree::get_trees(const corpus::document& doc)
{
    if (doc.contains_content())
        return content_trees(doc);
    return file_trees(doc);
}

std::vector<parse_tree> parse_tree::content_trees(const corpus::document& doc)
{
    std::vector<parse_tree> trees;
    std::istringstream stream{doc.content()};
    std::string cur;
    while (stream >> cur)
        trees.emplace_back(cur);
    return trees;
}

std::vector<parse_tree> parse_tree::file_trees(const corpus::document& doc)
{
    std::vector<parse_tree> trees;
    std::string filename{doc.path() + ".tree"};
    std::ifstream treefile{filename, std::ifstream::in};
    if (treefile.is_open())
    {
        std::string line;
        while (treefile >> line)
            trees.emplace_back(line);
        treefile.close();
    }
    else
        throw parse_tree_exception("failed to open " + filename);

    return trees;
}

uint64_t parse_tree::height(const parse_tree& curr)
{
    uint64_t max = 0;
    for (auto& child : curr.children())
    {
        uint64_t h = height(child) + 1;
        if (h > max)
            max = h;
    }
    return max;
}

std::string parse_tree::get_skeleton_children() const
{
    std::string ret{""};
    for (uint64_t i = 0; i < children_.size(); ++i)
        ret += "()";
    return ret;
}

std::string parse_tree::yield() const
{
    std::string result{""};
    for(auto& child: children_)
    {
        if(child.num_children() == 0)
            result += child.get_category() + " ";
        else
            result += child.yield();
    }

    return result;
}

}
}
