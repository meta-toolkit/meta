/**
 * @file ngram_distribution.cpp
 * Implementation of a smoothed ngram language model class.
 */

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;

typedef unordered_map<string, unsigned int> FreqMap;
typedef unordered_map<string, double> ProbMap;

template <size_t N>
NgramDistribution<N>::NgramDistribution(const string & docPath):
    _freqs(FreqMap()),
    _dist(ProbMap()),
    _lower(NgramDistribution<N - 1>(docPath))
{
    calc_freqs(docPath);
    calc_discount_factor();
    calc_dist();

    for(auto & n: _freqs)
        cout << n.first << " " << n.second << endl;
}

template <size_t N>
void NgramDistribution<N>::calc_discount_factor()
{
    size_t n1 = 0;
    size_t n2 = 0;

    for(auto & p: _freqs)
    {
        if(p.second == 1)
            ++n1;
        else if(p.second == 2)
            ++n2;
    }

    _discount = static_cast<double>(n1) / (n1 + 2 * n2);
}

template <size_t N>
void NgramDistribution<N>::calc_freqs(const string & docPath)
{
    Document doc(docPath);
    NgramTokenizer tokenizer(N, NgramTokenizer::Word,
        NgramTokenizer::NoStemmer, NgramTokenizer::NoStopwords);
    tokenizer.tokenize(doc);

    auto idFreqs = doc.getFrequencies();
    for(auto & p: idFreqs)
        _freqs.insert(std::make_pair(tokenizer.getLabel(p.first), p.second));
}

template <size_t N>
void NgramDistribution<N>::calc_dist()
{
    
}
