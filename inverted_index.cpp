/**
 * @file inverted_index.cpp
 */

#include "inverted_index.h"

InvertedIndex::InvertedIndex(const string & lexiconFile, const string & postingsFile):
    _lexicon(lexiconFile),
    _postings(postingsFile)
{
    /* nothing */
}

size_t InvertedIndex::getAvgDocLength() const
{
    return 0;
}

multimap<double, string> InvertedIndex::search(const Document & query) const
{
    // figure out which ranking algorithm to use; for now we just have Okapi
    multimap<double, string> results;
    return results;
}

string InvertedIndex::classifyKNN(const Document & query, size_t k) const
{
    return "chuck testa";
}
