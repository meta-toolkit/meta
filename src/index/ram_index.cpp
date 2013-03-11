/**
 * @file ram_index.cpp
 */

#include <cmath>
#include <iostream>
#include "tokenizers/tokenizer.h"
#include "index/ram_index.h"
#include "index/document.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::multimap;
using std::unordered_map;

RAMIndex::RAMIndex(const vector<string> & indexFiles, std::shared_ptr<Tokenizer> tokenizer):
    _tokenizer(tokenizer),
    _documents(vector<Document>()),
    _docFreqs(new unordered_map<TermID, unsigned int>),
    _avgDocLength(0)
{
    cout << "[RAMIndex]: creating index from " << indexFiles.size() << " documents" << endl;
    
    size_t docNum = 0;
    for(auto & file: indexFiles)
    {
        Document document(file);
        _tokenizer->tokenize(document, _docFreqs);
        _documents.push_back(document);
        _avgDocLength += document.getLength();

        if(docNum++ % 10 == 0)
            cout << "  " << ((double) docNum / indexFiles.size() * 100) << "%    \r";
    }
    cout << "  100%        " << endl;

    _avgDocLength /= _documents.size();
}

RAMIndex::RAMIndex(const vector<Document> & indexDocs, std::shared_ptr<Tokenizer> tokenizer):
    _tokenizer(tokenizer),
    _documents(indexDocs),
    _docFreqs(new unordered_map<TermID, unsigned int>),
    _avgDocLength(0)
{
    cout << "[RAMIndex]: creating index from " << indexDocs.size() << " documents" << endl;

    size_t docNum = 0;
    for(auto & doc: _documents)
    {
        //combineMap(doc.getFrequencies()); // call this if doc was already tokenized
        _tokenizer->tokenize(doc, _docFreqs);
        _avgDocLength += doc.getLength();
        if(docNum++ % 10 == 0)
        {
            cout << "  " << ((double) docNum / _documents.size() * 100) << "%    \r";
            cout.flush();
        }
    }
    cout << "  100%        " << endl;

    _avgDocLength /= _documents.size();
}

void RAMIndex::combineMap(const unordered_map<TermID, unsigned int> & newFreqs)
{
    for(auto & freq: *_docFreqs)
        (*_docFreqs)[freq.first] += freq.second;
}

double RAMIndex::scoreDocument(const Document & document, const Document & query) const
{
    // seems horrible
    bool bm25L = false;

    double score = 0.0;
    double k1 = 1.5; // 1.5 -> 2.5
    double b = 0.75; // 0.75 -> 1.0
    double k3 = 500;
    double delta = 0.5;
    double docLength = document.getLength();
    double numDocs = _documents.size();

    const unordered_map<TermID, unsigned int> frequencies = query.getFrequencies();
    for(auto & term: frequencies)
    {
        auto df = _docFreqs->find(term.first);
        double docFreq = (df == _docFreqs->end()) ? (0.0) : (df->second);
        double termFreq = document.getFrequency(term.first);
        double queryTermFreq = query.getFrequency(term.first);

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

multimap<double, string> RAMIndex::search(Document & query) const
{
    _tokenizer->tokenize(query);
    multimap<double, string> ranks;
    for(size_t idx = 0; idx < _documents.size(); ++idx)
    {
        double score = scoreDocument(_documents[idx], query);
        if(score != 0.0)
        {
            ranks.insert(make_pair(score, _documents[idx].getName() + " " + _documents[idx].getCategory()));
        }
    }

    return ranks;
}

void RAMIndex::indexDocs(std::vector<Document> & documents, size_t chunkMBSize)
{
    // put code for index creation in here?
}
