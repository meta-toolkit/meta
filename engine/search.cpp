#include "engine.h"

/**
 * Scores a document given a query.
 * @param document - the doc to score
 * @param query - the query to score against
 * @return the real score value 
 */
double engine::search::scoreDocument(const Document & document, const Document & query)
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
