/**
 * @file ngram_distribution.tcc
 * Implementation of a smoothed ngram language model class.
 */

#include <stdexcept>
#include "stemmers/no_stemmer.h"
#include "tokenizers/ngram/ngram_word_tokenizer.h"

namespace meta
{
namespace language_model
{

typedef std::unordered_map<std::string,
                           std::unordered_map<std::string, size_t>> FreqMap;
typedef std::unordered_map<std::string,
                           std::unordered_map<std::string, double>> ProbMap;

template <size_t N>
double ngram_distribution
    <N>::log_likelihood(const index::document& document) const
{
    return 0.0;
}

template <size_t N>
double ngram_distribution<N>::perplexity(const index::document& document) const
{
    return 0.0;
}

template <size_t N>
std::string ngram_distribution
    <N>::random_sentence(unsigned int seed, size_t num_words) const
{
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> rdist(0.0, 1.0);

    // create the beginning of the sentence
    std::string sentence = get_prev(rdist(gen));

    // create initial deque
    std::deque<std::string> ngram;
    std::string buff(sentence);
    for (size_t i = 0; i < N - 1; ++i)
    {
        ngram.push_front(get_last(buff));
        buff = get_rest(buff);
    }

    // append likely words, creating a sentence
    std::string word = get_word(rdist(gen), dist_.at(sentence));
    for (size_t i = 0; i < num_words; ++i)
    {
        ngram.push_back(word);
        if (word == "-rrb-")
            word = ")";
        else if (word == "-lrb-")
            word = "(";
        else if (word == "</s>")
            word = "\n\n";
        else if (word == "<s>")
            word = "";
        sentence += " " + word;
        ngram.pop_front();
        std::string prev = to_prev(ngram);
        auto it = dist_.find(prev);
        word = get_word(rdist(gen), it->second);
    }

    return sentence;
}

template <size_t N>
std::string ngram_distribution
    <N>::to_prev(const std::deque<std::string>& ngram) const
{
    std::string ret = "";
    for (auto& w : ngram)
        ret += " " + w;
    return ret.substr(1);
}

template <size_t N>
std::string ngram_distribution<N>::get_prev(double rand) const
{
    double range = 0.0;
    for (auto& d : dist_)
        range += d.second.size();
    rand *= range;

    double sum = 0.0;
    for (auto& d : dist_)
    {
        if (sum > rand)
            return d.first;
        sum += d.second.size();
    }

    return "";
}

template <size_t N>
std::string ngram_distribution
    <N>::get_word(double rand,
                  const std::unordered_map<std::string, double>& dist) const
{
    double range = 0.0;
    for (auto& prob : dist)
        range += prob.second;
    rand *= range;

    double sum = 0.0;
    for (auto& prob : dist)
    {
        sum += prob.second;
        if (sum > rand || prob.second > rand)
            return prob.first;
    }

    return "";
}

template <size_t N>
const ProbMap& ngram_distribution<N>::kth_distribution(size_t k) const
{
    if (k == 0)
        throw std::out_of_range("kth_distribution value is 0");

    if (k == 1)
        return dist_;

    return lower_.kth_distribution(k - 1);
}

template <size_t N>
ngram_distribution<N>::ngram_distribution(const std::string& doc_path)
    : freqs_(FreqMap()),
      dist_(ProbMap()),
      lower_(ngram_distribution<N - 1>(doc_path))
{
    std::cerr << " Creating " << N << "-gram model..." << std::endl;
    calc_freqs(doc_path);
    calc_discount_factor();
    calc_dist();
}

template <size_t N>
void ngram_distribution<N>::calc_discount_factor()
{
    size_t n1 = 0;
    size_t n2 = 0;

    for (auto& pmap : freqs_)
    {
        for (auto& wmap : pmap.second)
        {
            if (wmap.second == 1)
                ++n1;
            else if (wmap.second == 2)
                ++n2;
        }
    }

    discount_ = static_cast<double>(n1) / (n1 + 2 * n2);
}

template <size_t N>
void ngram_distribution<N>::calc_freqs(const std::string& doc_path)
{
    using namespace tokenizers;
    index::document doc(doc_path);
    ngram_word_tokenizer
        <stemmers::no_stemmer> tok(N, ngram_word_traits::NoStopwords);
    tok.tokenize(doc);

    for (auto& p : doc.frequencies())
    {
        std::string str = tok.label(p.first);
        std::string word = get_last(str);
        std::string rest = get_rest(str);
        std::remove(word.begin(), word.end(), ' ');
        freqs_[rest][word] += p.second;
    }
}

template <size_t N>
std::string ngram_distribution<N>::get_last(const std::string& words) const
{
    size_t idx = words.find_last_of(" ");
    return words.substr(idx + 1);
}

template <size_t N>
std::string ngram_distribution<N>::get_rest(const std::string& words) const
{
    size_t idx = words.find_last_of(" ");
    if (idx == std::string::npos)
        return "";
    return words.substr(0, idx);
}

template <size_t N>
void ngram_distribution<N>::calc_dist()
{
    for (auto& pmap : freqs_)
    {
        // string of previous words up to w_n
        std::string prev = pmap.first;

        // how many times prev appears
        size_t c_prev = 0;
        for (auto& wmap : pmap.second)
            c_prev += wmap.second;

        // how many different prev there are
        size_t s_w = pmap.second.size();

        for (auto& wmap : pmap.second)
        {
            std::string word = wmap.first;
            size_t c_prevw = wmap.second;
            dist_[prev][word] = std::max(c_prevw - discount_, 0.0) / c_prev;
            dist_[prev][word] += (discount_ / c_prev) * s_w * lower_.prob(prev);
        }
    }
}

template <size_t N>
double ngram_distribution<N>::prob(const std::string& str) const
{
    std::string word = get_last(str);
    std::string prev = get_rest(str);
    return dist_.at(prev).at(word);
}

template <size_t N>
double ngram_distribution<N>::prob(const std::string& prev,
                                   const std::string& word) const
{
    return dist_(prev).at(word);
}
}
}
