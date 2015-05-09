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
#include <unordered_map>
#include <string>
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
    language_model(const cpptoml::table& config);

    /**
     * @param sentence A sequence of tokens
     * @return the perplexity of this token sequence given the current language
     * model: \f$ \sqrt[n]{\prod_{i=1}^n\frac{1}{p(w_i|w_{i-n}\cdots w_{i-1})}}
     * \f$
     */
    float perplexity(const sentence& tokens) const;

    /**
     * @param sentence A sequence of tokens
     * @return the perplexity of this token sequence given the current language
     * model normalized by the length of the sequence
     */
    float perplexity_per_word(const sentence& tokens) const;

    /**
     * @param tokens A sequence of n tokens (one sentence)
     * @return the log probability of the likelihood of this sentence
     */
    float log_prob(sentence tokens) const;

    /**
     * @param prev Seen tokens to base the next token off of
     * @param k Number of results to return
     * @return a sorted vector of likely next tokens
     */
    std::vector<std::pair<std::string, float>> top_k(const sentence& prev,
                                                      size_t k) const;

  private:
    /**
     * Reads precomputed LM data into this object.
     * @param arpa_file The path to the ARPA-formatted file
     */
    void read_arpa_format(const std::string& arpa_file);

    /**
     * @param tokens
     * @return the log probability of one ngram
     */
    float prob_calc(sentence tokens) const;

    uint64_t N_; /// The "n" value for this n-gram language model

    /**
     * Simple struct to keep track of probabilities and backoff values.
     */
    struct lm_node
    {
        lm_node():
            prob{0.0f}, backoff{0.0f} {}

        lm_node(float p, float b):
            prob{p}, backoff{b} {}

        float prob;
        float backoff;
    };

    std::vector<std::unordered_map<std::string, lm_node>> lm_;
};

class language_model_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
}

#endif
