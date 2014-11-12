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

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "cpptoml.h"
#include "lm/sentence.h"

namespace meta
{
namespace lm
{
class language_model
{
  public:
    /**
     * Creates an N-gram language model based on the corpus specified in the
     * config file.
     */
    language_model(const cpptoml::toml_group& config);

    /**
     * Creates an N-gram language model based on the corpus specified in the
     * config file.
     * @param n The value of n, which overrides any setting in the config file
     */
    language_model(const cpptoml::toml_group& config, size_t n);

    /**
     * Randomly generates one token sequence based on <s> and </s> symbols.
     * @return a random sequence of tokens based on this language model
     */
    std::string generate(unsigned int seed) const;

    /**
     * @param sentence The previous N - 1 tokens
     * @param random A random number on [0, 1] used for choosing the next token
     * @return the next token based on the previous tokens
     */
    std::string next_token(const sentence& sen, double random) const;

    /**
     * @param sentence A sequence of tokens
     * @return the perplexity of this token sequence given the current language
     * model: \f$ \sqrt[n]{\prod_{i=1}^n\frac{1}{p(w_i|w_{i-n}\cdots w_{i-1})}}
     * \f$
     */
    double perplexity(const sentence& tokens) const;

    /**
     * @param sentence A sequence of tokens
     * @return the perplexity of this token sequence given the current language
     * model normalized by the length of the sequence
     */
    double perplexity_per_word(const sentence& tokens) const;

    /**
     * @param tokens A sequence of n tokens
     * @return the probability of seeing the nth token based on the previous n
     * - 1 tokens
     */
    double prob(sentence tokens) const;

    /**
     * @param prev Seen tokens to base the next token off of
     * @param k Number of results to return
     * @return a sorted vector of likely next tokens
     */
    std::vector<std::pair<std::string, double>> top_k(const sentence& prev,
                                                      size_t k) const;

  private:
    /**
     * Builds the probabilities associated with this language model.
     * @param config The config file that specifies the location of the
     * corpus
     */
    void learn_model(const cpptoml::toml_group& config);

    /**
     * @param config
     */
    void select_method(const cpptoml::toml_group& config);

    /**
     * @param prefix Path to where the counts files are stored
     */
    void read_precomputed(const std::string& prefix);

    /// The language_model used to interpolate with this one for smoothing
    std::shared_ptr<language_model> interp_; // shared to allow copying

    /// Contains the N-gram distribution probabilities (N-1 words -> (w, prob))
    std::unordered_map<std::string, std::unordered_map<std::string, double>>
        dist_;

    /// The value of N in this n-gram
    size_t N_;

    /// The interpolation coefficient for smoothing LM probabilities
    constexpr static double lambda_ = 0.7;
};

class language_model_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
}

#endif

