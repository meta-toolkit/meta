#include "ram_index.h"

RAMIndex::RAMIndex(const vector<string> & indexFiles,
                   const Tokenizer* tokenizer)
{
    cout << "Creating RAM index" << endl;

    _documents = vector<Document>();
    _avgDocLength = 0;
    
    // get a vector of parse trees for each file
    for(vector<string>::const_iterator file = indexFiles.begin(); file != indexFiles.end(); ++file)
    {
        Document document(*file, "N/A");
        unordered_map<string, size_t> tokens = tokenizer->getTokens(*file);
        for(unordered_map<string, size_t>::const_iterator token = tokens.begin(); token != tokens.end(); ++token)
            document.increment(token->first, token->second);

        _documents.push_back(document);
        _avgDocLength += document.getLength();
    }

    _avgDocLength /= _documents.size();
}

double RAMIndex::scoreDocument(const Document & document, const Document & query) const
{
    double score = 0.0;

    const unordered_map<string, size_t> frequencies = query.getFrequencies();
    for(unordered_map<string, size_t>::const_iterator term = frequencies.begin(); term != frequencies.end(); ++term)
    {
        double k1 = 1.5;
        double b = 0.75;
        double k3 = 500;

        double numDocs = _documents.size();
        double docFreq = 1; // ugh, this is why an inverted index is nice....
        double termFreq = document.getFrequency(term->first);
        double queryTermFreq = query.getFrequency(term->first);

        double IDF = log((numDocs - docFreq + 0.5) / (docFreq + 0.5));
        double TF = ((k1 + 1.0) * termFreq) / ((k1 * ((1.0 - b) + b * document.getLength() / _avgDocLength)) + termFreq);
        double QTF = ((k3 + 1.0) * queryTermFreq) / (k3 + queryTermFreq);

        score += IDF * TF * QTF;
    }

    return score;
}


size_t RAMIndex::getAvgDocLength() const
{
    return _avgDocLength;
}


multimap<double, string> RAMIndex::search(const Document & query) const
{
    cout << "Scoring documents" << endl;

    // score documents
    multimap<double, string> ranks;
    for(vector<Document>::const_iterator doc = _documents.begin(); doc != _documents.end(); ++doc)
    {
        double score = scoreDocument(*doc, query);
        ranks.insert(make_pair(score, doc->getAuthor()));
    }

    return ranks; // seems like a bad idea
}
