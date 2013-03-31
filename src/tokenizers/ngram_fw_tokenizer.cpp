/**
 * @file ngram_fw_tokenizer.cpp
 */

#include "io/parser.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_fw_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::deque;
using std::string;
using std::unordered_map;
using std::unordered_set;

using index::TermID;
using index::Document;
using io::Parser;

ngram_fw_tokenizer::ngram_fw_tokenizer(size_t n):
    ngram_tokenizer(n),
    _function_words(unordered_set<string>())
{
    init_function_words();
}

void ngram_fw_tokenizer::tokenize(Document & document,
        const std::shared_ptr<unordered_map<TermID, unsigned int>> & docFreq)
{
    Parser parser(document.getPath() + ".sen", " \n");

    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < n_value() && parser.hasNext(); ++i)
    {
        string next = "";
        do
        {
            next = parser.next();
        } while(_function_words.find(next) == _function_words.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(mapping(wordified), 1, docFreq);
        ngram.pop_front();
        string next = "";
        do
        {
            next = parser.next();
        } while(_function_words.find(next) == _function_words.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the last token
    document.increment(mapping(wordify(ngram)), 1, docFreq);
}

void ngram_fw_tokenizer::init_function_words()
{
    auto config = io::config_reader::read("tokenizer.ini");
    Parser parser(config["function-words"], " \n");
    while(parser.hasNext())
        _function_words.insert(parser.next());
}

}
}
