#include "ram_index.h"

RAMIndex::RAMIndex(const vector<string> & indexFiles,
                   const Tokenizer* tokenizer)
{
    cout << "[RAMIndex]: creating index from " << indexFiles.size() << " files" << endl;

    _docFreqs = unordered_map<string, size_t>();
    _documents = vector<Document>();
    _avgDocLength = 0;
    
    size_t docNum = 0;
    for(vector<string>::const_iterator file = indexFiles.begin(); file != indexFiles.end(); ++file)
    {
        Document document(getName(*file), getCategory(*file));
        tokenizer->tokenize(*file, document, &_docFreqs);
        _documents.push_back(document);
        _avgDocLength += document.getLength();

        if(docNum++ % 10 == 0)
            cout << "  " << ((double) docNum / indexFiles.size() * 100) << "%    \r";
    }
    cout << "  100%        " << endl;

    _avgDocLength /= _documents.size();
}

string RAMIndex::getName(const string & path)
{
    size_t idx = path.find_last_of("/") + 1;
    return path.substr(idx, path.size() - idx);
}

string RAMIndex::getCategory(const string & path)
{
    size_t idx = path.find_last_of("/");
    string sub = path.substr(0, idx);
    return getName(sub);
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
        unordered_map<string, size_t>::const_iterator df = _docFreqs.find(term->first);
        double docFreq = (df == _docFreqs.end()) ? (0.0) : (df->second);
        double termFreq = document.getFrequency(term->first);
        double queryTermFreq = query.getFrequency(term->first);

        double IDF = log((numDocs - docFreq + 0.5) / (docFreq + 0.5));
        double TF = ((k1 + 1.0) * termFreq) / ((k1 * ((1.0 - b) + b * document.getLength() / _avgDocLength)) + termFreq);
        double QTF = ((k3 + 1.0) * queryTermFreq) / (k3 + queryTermFreq);

        //cout << "IDF = " << IDF << ", TF = " << TF << ", QTF = " << QTF << endl;
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
    cout << "[RAMIndex]: scoring documents for query " << query.getName()
         << " (" << query.getCategory() << ")" << endl;

    // score documents
    multimap<double, string> ranks;
    for(vector<Document>::const_iterator doc = _documents.begin(); doc != _documents.end(); ++doc)
    {
        double score = scoreDocument(*doc, query);
        if(score != 0.0)
            ranks.insert(make_pair(score, doc->getName() + " (" + doc->getCategory() + ")"));
    }

    return ranks; // seems like a bad idea
}

/**
 * Classify the query document by category using K-Nearest Neighbor.
 * @param query - the query to run
 * @param k - the value of k in KNN
 * @return the category the document is believed to be in
 */
string RAMIndex::classifyKNN(const Document & query, size_t k) const
{
    multimap<double, string> ranking = search(query);
    unordered_map<string, size_t> counts;
    size_t numResults = 0;
    for(multimap<double, string>::reverse_iterator result = ranking.rbegin(); result !=
      ranking.rend() && numResults++ != k; ++result)
    {
        size_t space = result->second.find_first_of(" ") + 1;
        string category = result->second.substr(space, result->second.size() - space);
        counts[category]++;
    }

    // sort
    multimap<size_t, string> results;
    for(unordered_map<string, size_t>::iterator it = counts.begin(); it != counts.end(); ++it)
        results.insert(make_pair(it->second, it->first));
    return results.begin()->second;
}

