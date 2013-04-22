/**
 * @file ram_index.cpp
 */

#include <cmath>
#include <iostream>
#include "index/ram_index.h"
#include "index/document.h"

namespace meta {
namespace index {

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::multimap;
using std::unordered_map;

using tokenizers::tokenizer;

RAMIndex::RAMIndex(const vector<document> & indexDocs, std::shared_ptr<tokenizer> tokenizer):
    _tokenizer(tokenizer),
    _documents(indexDocs),
    _docFreqs(new unordered_map<term_id, unsigned int>),
    _avgDocLength(0)
{
    cout << "[RAMIndex]: creating index from " << indexDocs.size() << " documents" << endl;

    size_t docNum = 0;
    for(auto & doc: _documents)
    {
        _tokenizer->tokenize(doc, _docFreqs);
        _avgDocLength += doc.length();
        if(docNum++ % 10 == 0)
        {
            cout << "  " << ((double) docNum / _documents.size() * 100) << "%    \r";
            cout.flush();
        }
    }
    cout << "  100%        " << endl;

    _avgDocLength /= _documents.size();
}

double RAMIndex::score_document(const document & doc, const document & query) const
{
    // seems horrible
    bool bm25L = false;

    double score = 0.0;
    double k1 = 1.5; // 1.5 -> 2.5
    double b = 0.75; // 0.75 -> 1.0
    double k3 = 500;
    double delta = 0.5;
    double docLength = doc.length();
    double numDocs = _documents.size();

    const unordered_map<term_id, unsigned int> & frequencies = query.frequencies();
    for(auto & term: frequencies)
    {
        auto df = _docFreqs->find(term.first);
        double docFreq = (df == _docFreqs->end()) ? (0.0) : (df->second);
        double termFreq = doc.frequency(term.first);
        double queryTermFreq = query.frequency(term.first);

        //double IDF = (numDocs - docFreq + 0.5) / (docFreq + 0.5);
        double IDF = log(1 + ((numDocs - docFreq + 0.5) / (docFreq + 0.5)));
        double TF = 0;

        if(bm25L)
        {
            double cPrime = termFreq / (1 - b + b * docLength / _avgDocLength);
            if(cPrime > 0)
                TF = ((k1 + 1) * (cPrime + delta)) / (k1 * (cPrime + delta));
        }
        else
            TF = ((k1 + 1.0) * termFreq) / ((k1 * ((1.0 - b) + b * docLength / _avgDocLength)) + termFreq);

        double QTF = ((k3 + 1.0) * queryTermFreq) / (k3 + queryTermFreq);

        score += IDF * TF * QTF;
    }

    return score;
}

size_t RAMIndex::getAvgDocLength() const
{
    return _avgDocLength;
}

multimap<double, string> RAMIndex::search(document & query) const
{
    _tokenizer->tokenize(query);
    multimap<double, string> ranks;
    for(size_t idx = 0; idx < _documents.size(); ++idx)
    {
        double score = score_document(_documents[idx], query);
        if(score != 0.0)
        {
            ranks.insert(make_pair(score, _documents[idx].name() + " " + _documents[idx].label()));
        }
    }

    return ranks;
}

}
}
