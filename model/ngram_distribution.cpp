/**
 * @file ngram_distribution.cpp
 * Implementation of a smoothed ngram language model class.
 */

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;

typedef unordered_map<string, unordered_map<string, size_t>> FreqMap;
typedef unordered_map<string, unordered_map<string, double>> ProbMap;

template <size_t N>
NgramDistribution<N>::NgramDistribution(const string & docPath):
    _freqs(FreqMap()),
    _dist(ProbMap()),
    _lower(NgramDistribution<N - 1>(docPath))
{
    calc_freqs(docPath);
    calc_discount_factor();
    calc_dist();
    cout << N << " constructor done" << endl;
}

template <size_t N>
void NgramDistribution<N>::calc_discount_factor()
{
    size_t n1 = 0;
    size_t n2 = 0;

    for(auto & pmap: _freqs)
    {
        for(auto & wmap: pmap.second)
        {
            if(wmap.second == 1)
                ++n1;
            else if(wmap.second == 2)
                ++n2;
        }
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

    for(auto & p: doc.getFrequencies())
    {
        string str = tokenizer.getLabel(p.first);
        string word = get_last(str);
        string rest = get_rest(str);
        _freqs[rest][word] += p.second;
    }
}

template <size_t N>
string NgramDistribution<N>::get_last(const string & words) const
{
    size_t idx = words.find_last_of(" ");
    return words.substr(idx + 1);
}

template <size_t N>
string NgramDistribution<N>::get_rest(const string & words) const
{
    size_t idx = words.find_last_of(" ");
    if(idx == string::npos)
        return "";
    return words.substr(0, idx);
}

template <size_t N>
void NgramDistribution<N>::calc_dist()
{
    for(auto & pmap: _freqs)
    {
        // string of previous words up to w_n
        string prev = pmap.first;

        // how many times prev appears
        size_t c_prev = 0;
        for(auto & wmap: pmap.second)
            c_prev += wmap.second;

        // how many different prev there are
        size_t s_w = pmap.second.size();
        
        for(auto & wmap: pmap.second)
        {
            string word = wmap.first;
            size_t c_prevw = wmap.second;
            _dist[prev][word] = std::max(c_prevw - _discount, 0.0) / c_prev;
            _dist[prev][word] += (_discount / c_prev) * s_w * _lower.prob(prev);
            
            //  _dist[prev][word] = static_cast<double>(c_prevw) / c_prev; // unsmoothed

            // cout << " p(" << word << "|" << prev << ") = " << _dist[prev][word] << endl;
        }
    }
}

template <size_t N>
double NgramDistribution<N>::prob(const string & str) const
{
    string word = get_last(str);
    string prev = get_rest(str);
    return _dist.at(prev).at(word);
}

template <size_t N>
double NgramDistribution<N>::prob(const string & prev, const string & word) const
{
    return _dist(prev).at(word);
}
