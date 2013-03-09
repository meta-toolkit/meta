/**
 * @file ngram_distribution.tcc
 * Implementation of a smoothed ngram language model class.
 */

#include <vector>
#include <stdexcept>

using std::deque;
using std::vector;
using std::endl;
using std::string;
using std::unordered_map;

typedef unordered_map<string, unordered_map<string, size_t>> FreqMap;
typedef unordered_map<string, unordered_map<string, double>> ProbMap;

template <size_t N>
double NgramDistribution<N>::log_likelihood(const Document & document) const
{
    return 0.0;
}

template <size_t N>
double NgramDistribution<N>::perplexity(const Document & document) const
{
    return 0.0;
}

template <size_t N>
string NgramDistribution<N>::random_sentence(unsigned int seed, size_t numWords) const
{
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> rdist(0.0, 1.0);

    // create the beginning of the sentence
    string sentence = get_prev(rdist(gen));

    // create initial deque
    deque<string> ngram;
    string buff(sentence);
    for(size_t i = 0; i < N - 1; ++i)
    {
        ngram.push_front(get_last(buff));
        buff = get_rest(buff);
    }

    // append likely words, creating a sentence
    string word = get_word(rdist(gen), _dist.at(sentence));
    for(size_t i = 0; i < numWords; ++i)
    {
        ngram.push_back(word);
        if(word == "-rrb-")
            word = ")";
        else if(word == "-lrb-")
            word = "(";
        else if(word == "</s>")
            word = "\n\n";
        else if(word == "<s>")
            word = "";
        sentence += " " + word;
        ngram.pop_front();
        string prev = to_prev(ngram);
        auto it = _dist.find(prev);
        word = get_word(rdist(gen), it->second);
    }

    return sentence;
}

template <size_t N>
string NgramDistribution<N>::to_prev(const deque<string> & ngram) const
{
    string ret = "";
    for(auto & w: ngram)
        ret += " " + w;
    return ret.substr(1);
}

template <size_t N>
string NgramDistribution<N>::get_prev(double rand) const
{
    double range = 0.0;
    for(auto & d: _dist)
        range += d.second.size();
    rand *= range;

    double sum = 0.0;
    for(auto & d: _dist)
    {
        if(sum > rand)
            return d.first;
        sum += d.second.size();
    }

    return "";
}

template <size_t N>
string NgramDistribution<N>::get_word(double rand, const unordered_map<string, double> & dist) const
{
    double range = 0.0;
    for(auto & prob: dist)
        range += prob.second;
    rand *= range;

    double sum = 0.0;
    for(auto & prob: dist)
    {
        sum += prob.second;
        if(sum > rand || prob.second > rand)
            return prob.first;
    }

    return "";
}

template <size_t N>
const ProbMap & NgramDistribution<N>::kth_distribution(size_t k) const
{
    if(k == 0)
        throw std::out_of_range("kth_distribution value is 0");

    if(k == 1)
        return _dist;

    return _lower.kth_distribution(k - 1);
}

template <size_t N>
NgramDistribution<N>::NgramDistribution(const string & docPath):
    _freqs(FreqMap()),
    _dist(ProbMap()),
    _lower(NgramDistribution<N - 1>(docPath))
{
    std::cerr << " Creating " << N << "-gram model..." << std::endl;
    calc_freqs(docPath);
    calc_discount_factor();
    calc_dist();
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
        std::remove(word.begin(), word.end(), ' ');
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
