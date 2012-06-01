#include "ram_index.h"

RAMIndex::RAMIndex(const vector<string> & indexFiles,
                   const ParseTreeTokenizer & tokenizer)
{
    cout << "Creating RAM index" << endl;

    _documents = vector<Document>();
    _avgDocLength = 0;
    
    // get a vector of parse trees for each file
    for(vector<string>::const_iterator file = indexFiles.begin(); file != indexFiles.end(); ++file)
    {
        vector<ParseTree> trees = ParseTree::getTrees(*file);
        Document document(*file, "N/A");
       
        // aggregate token counts for each tree
        for(vector<ParseTree>::const_iterator tree = trees.begin(); tree != trees.end(); ++tree)
            tokenizer.tokenize(*tree, document);
        _documents.push_back(document);
        _avgDocLength += document.getLength();
    }

    _avgDocLength /= _documents.size();
}

double RAMIndex::scoreDocument(const Document & document,
                               const Document & query) const
{
    double score = 0.0;

    const unordered_map<string, size_t> frequencies = query.getFrequencies();
    for(unordered_map<string, size_t>::const_iterator freq = frequencies.begin(); freq != frequencies.end(); ++freq)
    {
        double N = 0;
        double avgDL;
        double k1 = 1.5;
        double b = 1;
        double k3 = 500;

        double df = 1;
        double tf = document.getFrequency(freq->first);
        double qtf = query.getFrequency(freq->first);

        double IDF = log((N - df + 0.5) / (df + 0.5));
        double TF = ((k1 + 1.0) * tf) / ((k1 * ((1.0-b) + b * document.getLength() / avgDL)) + tf);
        double QTF = ((k3 + 1.0) * qtf) / (k3 + qtf);

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
