/**
 * @file language_model.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LANGUAGE_MODEL_H_
#define META_LANGUAGE_MODEL_H_

#include <string>
#include <deque>
#include <unordered_map>

namespace meta
{
namespace lm
{
template <size_t N>
class language_model
{
  public:
    /**
     * Creates an N-gram language model based on the corpus specified in the
     * config file.
     */
    language_model(const std::string& config_file);

    /**
     * Randomly generates one token sequence based on <s> and </s> symbols.
     * @return a random sequence of tokens based on this language model
     */
    std::string generate(unsigned int seed) const;

    /**
     * @param tokens The previous N - 1 tokens
     * @param random A random number on [0, 1] used for choosing the next token
     * @return the next token based on the previous tokens
     */
    std::string next_token(const std::deque<std::string>& tokens,
                           double random) const;

    /**
     * @param tokens A sequence of tokens
     * @return the perplexity of this token sequence given the current language
     * model: \f$ \sqrt[n]{\prod_{i=1}^n\frac{1}{p(w_i|w_{i-n}\cdots w_{i-1})}}
     * \f$
     */
    double perplexity(const std::string& tokens) const;

    /**
     * @param tokens A sequence of tokens
     * @return the perplexity of this token sequence given the current language
     * model normalized by the length of the sequence
     */
    double perplexity_per_word(const std::string& tokens) const;

    /**
     * @param tokens A sequence of n tokens
     * @return the probability of seeing the nth token based on the previous n
     * - 1 tokens
     */
    double prob(std::deque<std::string> tokens) const;

  private:
    /**
     * @param tokens A deque of tokens to convert to a string
     * @return the string version of the deque (space delimited)
     */
    std::string make_string(const std::deque<std::string>& tokens) const;

    /**
     * @param tokens A string of space-delimited tokens to convert to a deque
     * @return a deque of the tokens
     */
    std::deque<std::string> make_deque(const std::string& tokens) const;

    /// The language_model used to interpolate with this one for smoothing
    language_model<N - 1> interp_;

    /// Contains the N-gram distribution probabilities (N-1 words -> (w, prob))
    std::unordered_map
        <std::string, std::unordered_map<std::string, double>> dist_;

    /// The interpolation coefficient for smoothing LM probabilities
    constexpr static double lambda_ = 0.7;
};

template <>
class language_model<0>
{
  public:
    language_model(const std::string& config_file)
    {/* nothing */
    }

    std::string next_token(const std::deque<std::string>& tokens,
                           double random) const
    {
        throw std::runtime_error{
            "next_token should not be called on a 0-gram language model"};
    }

    double prob(std::deque<std::string> tokens) const
    {
        return 0.0;
    }

  private:
};
}
}

#include "language_model.tcc"
#endif

