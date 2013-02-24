/**
 * @file ngram_distribution.cpp
 */

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;

typedef unordered_map<string, unsigned int> FreqMap;

template <size_t N>
NgramDistribution<N>::NgramDistribution(const string & docPath):
    _dist(FreqMap())
{
    calc_freqs();
    calc_discount_factor();

    for(auto & n: _dist)
        cout << n.first << " " << n.second << endl;
}

template <size_t N>
void NgramDistribution<N>::calc_discount_factor()
{
    size_t n1 = 0;
    size_t n2 = 0;

    for(auto & p: _dist)
    {
        if(p.second == 1)
            ++n1;
        else if(p.second == 2)
            ++n2;
    }

    _discount = static_cast<double>(n1) / (n1 + 2 * n2);
}

template <size_t N>
void NgramDistribution<N>::calc_freqs()
{
    Document doc("train.txt");
    NgramTokenizer tokenizer(N, NgramTokenizer::Word,
        NgramTokenizer::NoStemmer, NgramTokenizer::NoStopwords);
    tokenizer.tokenize(doc);

    auto idFreqs = doc.getFrequencies();
    for(auto & p: idFreqs)
        _dist.insert(std::make_pair(tokenizer.getLabel(p.first), p.second));
}
