/**
 * @file inverted_index.cpp
 */

#include "inverted_index.h"

InvertedIndex::InvertedIndex(const string & lexiconFile):
    _lexicon(Lexicon(lexiconFile))
{
}

double InvertedIndex::scoreDocument(const Document & document, const Document & query) const
{
    return 0.0;
}

size_t InvertedIndex::getAvgDocLength() const
{
    return 0;
}

multimap<double, string> InvertedIndex::search(const Document & query) const
{
    multimap<double, string> results;
    return results;
}

string InvertedIndex::classifyKNN(const Document & query, size_t k) const
{
    return "chuck testa";
}
