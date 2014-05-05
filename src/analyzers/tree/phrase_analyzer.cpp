/**
 * @file phrase_analyzer.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iostream>
#include <fstream>
#include "analyzers/token_stream.h"
#include "analyzers/filters/alpha_filter.h"
#include "analyzers/filters/lowercase_filter.h"
#include "analyzers/tokenizers/whitespace_tokenizer.h"
#include "analyzers/filters/porter2_stemmer.h"
#include "analyzers/tree/phrase_analyzer.h"

namespace meta
{
namespace analyzers
{

const std::unordered_set<std::string> phrase_analyzer::clause_tags_
    = {"S", "SBAR", "SBARQ", "SINV", "SQ"};

const std::unordered_set<std::string> phrase_analyzer::phrase_tags_
    = {"ADJP", "ADVP", "CONJ",   "FRAG",  "INTJ", "LST",  "NAC",
       "NP",   "NX",   "PP",     "PRN",   "PRT",  "QP",   "RRC",
       "UCP",  "VP",   "WHADJP", "WHAVP", "WHNP", "WHPP", "X"};

const std::unordered_set<std::string> phrase_analyzer::pos_tags_ = {
    "CC",    "CD",    "DT",  "EX",  "FW",   "IN",  "JJ",  "JJR", "JJS",  "LS",
    "MD",    "NN",    "NNS", "NNP", "NNPS", "PDT", "POS", "PRP", "PRP$", "RB",
    "RBR",   "RBS",   "RP",  "SYM", "TO",   "UH",  "VB",  "VBD", "VBG",  "VBN",
    "VBP",   "VBZ",   "WDT", "WP",  "WP$",  "WRB", ".",   "!",   "?",    ",",
    "-RRB-", "-LRB-", "-",   "--",  "---",  "..."};

const std::string phrase_analyzer::id = "phrase";

void phrase_analyzer::tree_tokenize(corpus::document& doc,
                                    const parse_tree& tree)
{
    // see if all children are phrases
    /*
    auto children = tree.children();
    bool all_phrases
        = std::all_of(children.begin(), children.end(), [](const parse_tree& t)
    { return phrase_tags_.find(t.get_category()) != phrase_tags_.end(); });

    std::cout << tree.get_category() << ": " << tree.get_children_string() <<
    std::endl;
    if (all_phrases)
    {
        for (auto& child : children)
            tree_tokenize(doc, child);
    }
    else
    {
        phrases_.push_back(tree.yield());
    }
    */

    /*
    if (clause_tags_.find(tree.get_category()) == clause_tags_.end())
        phrases_.push_back(tree.yield());
    else
    {
        for (auto& child : tree.children())
            tree_tokenize(doc, child);
    }
    */

    auto yield = tree.yield();
    size_t yield_size = std::count(yield.begin(), yield.end(), ' ');
    if(yield_size <= 6)
        phrases_.push_back(tree.yield());
    else
    {
        for (auto& child : tree.children())
            tree_tokenize(doc, child);
    }
}

std::vector<std::string> phrase_analyzer::phrases()
{
    auto ret = phrases_;
    phrases_.clear();
    return ret;
}
}
}
