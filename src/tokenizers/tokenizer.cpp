/**
 * @file tokenizer.cpp
 */

#include <functional>
#include <fstream>
#include <iostream>
#include "util/invertible_map.h"
#include "tokenizers/tokenizer.h"

namespace meta {
namespace tokenizers {

using std::string;
using std::cout;
using std::endl;
using std::ofstream;
using namespace std::placeholders;
using std::unordered_map;
using util::invertible_map;
using index::document;

tokenizer::tokenizer():
    _term_map(invertible_map<term_id, string>()),
    _current_term_id(0)
{ /* nothing */ }

void tokenizer::tokenize(index::document & document,
        const std::shared_ptr<std::unordered_map<term_id, uint64_t>> & doc_freq) {
    tokenize_document(document, std::bind(&tokenizer::mapping, this, _1), doc_freq);
}

term_id tokenizer::mapping(const string & term)
{
    if(!_term_map.contains_value(term))
    {
        _term_map.insert(_current_term_id, term);
        return _current_term_id++;
    }
    else
    {
        term_id termID = _term_map.get_key(term);
        return termID;
    }
}

void tokenizer::set_term_id_mapping(const std::string & filename)
{
    _term_map.read(filename);
    _current_term_id = _term_map.size();
}

void tokenizer::save_term_id_mapping(const string & filename) const
{
    _term_map.save(filename);
}

const invertible_map<term_id, std::string> & tokenizer::term_id_mapping() const
{
    return _term_map;
}

string tokenizer::label(term_id termID) const
{
    return _term_map.get_value(termID);
}

void tokenizer::print_data() const
{
    for(auto & term: _term_map)
        cout << term.first << "\t" << term.second << endl;
}

void tokenizer::set_max_term_id(size_t start)
{
    _current_term_id = start;
}

term_id tokenizer::max_term_id() const
{
    return _current_term_id;
}

size_t tokenizer::num_terms() const
{
    return _term_map.size();

}

}
}
