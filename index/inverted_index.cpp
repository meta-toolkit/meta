/**
 * @file inverted_index.cpp
 */

#include "inverted_index.h"

InvertedIndex::InvertedIndex(const string & lexiconFile, const string & postingsFile, Tokenizer* tokenizer):
    _lexicon(lexiconFile),
    _postings(postingsFile),
    _tokenizer(tokenizer)
{ /* nothing */ }

multimap<double, string> InvertedIndex::search(const Document & query) const
{
    double k1 = 1.5;
    double b = 0.75;
    double k3 = 500;
    double numDocs = _lexicon.getNumDocs();
    double avgDL = _lexicon.getAvgDocLength();
    unordered_map<DocID, double> scores;

    // loop through each term in the query
    const unordered_map<TermID, unsigned int> freqs = query.getFrequencies();
    for(auto & queryTerm: freqs)
    {
        // loop through each document containing the current term
        TermID queryTermID = queryTerm.first;
        size_t queryTermFreq = queryTerm.second;
        TermData termData = _lexicon.getTermInfo(queryTermID);
        vector<PostingData> docList = _postings.getDocs(termData);
        for(auto & doc: docList)
        {
            double docLength = _lexicon.getDocLength(doc.docID);
            double IDF = log((numDocs - termData.idf + 0.5) / (termData.idf + 0.5));
            double TF = ((k1 + 1.0) * doc.freq) / ((k1 * ((1.0 - b) + b * docLength / avgDL)) + doc.freq);
            double QTF = ((k3 + 1.0) * queryTermFreq) / (k3 + queryTermFreq);
            double score = TF * IDF * QTF;
            scores[doc.docID] += score;
        }
    }

    // combine into sorted multimap
    multimap<double, string> results;
    for(auto & score: scores)
        results.insert(make_pair(score.second, _lexicon.getTerm(score.first)));
    return results;
}

string InvertedIndex::classifyKNN(const Document & query, size_t k) const
{
    return "chuck testa";
}

bool InvertedIndex::indexDocs(const vector<Document> & documents, size_t chunkMBSize)
{
    if(!_lexicon.isEmpty())
    {
        cerr << "[InvertedIndex]: attempted to create an index in an existing index location" << endl;
        return false;
    }

    size_t numChunks = _postings.createChunks(documents, chunkMBSize, _tokenizer);
    _postings.createPostingsFile(numChunks);
    _lexicon.createFromPostings("myindex.lexicon");

    return true;
}
