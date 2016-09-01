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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/lm/lm_state.h"
#include "meta/lm/sentence.h"
#include "meta/lm/static_probe_map.h"
#include "meta/lm/token_list.h"

namespace meta
{
namespace lm
{
class language_model_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/**
 * A very simple language model class that reads existing language model data
 * from a .arpa file. Currently, estimation of the language model is not
 * implemented. We recommend using KenLM to generate a .arpa file from a text
 * corpus that has been (optionally) preprocessed by MeTA.
 *
 * @see http://www.speech.sri.com/projects/srilm/manpages/ngram-format.5.html
 * @see https://kheafield.com/code/kenlm/
 *
 * Required config parameters:
 * ~~~toml
 * [language-model]
 * binary-file-prefix = "path-to-binary-files"
 * # only this key is needed if the LM is already binarized
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [language-model]
 * arpa-file = "path-to-arpa-file" # if no binary files have yet been created
 * ~~~
 */
class language_model
{
  public:
    /**
     * Creates an N-gram language model based on the corpus specified in the
     * config file.
     */
    language_model(const cpptoml::table& config);

    /**
     * Default move constructor.
     */
    language_model(language_model&&) = default;

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
    float log_prob(const sentence& tokens) const;

    /**
     * @param prev Seen tokens to base the next token off of
     * @param k Number of results to return
     * @return a sorted vector of likely next tokens
     * Complexity is currently O(|V|) due to the LM structure; this should be
     * changed in a future version of MeTA.
     */
    std::vector<std::pair<std::string, float>> top_k(const sentence& prev,
                                                     size_t k) const;

    /**
     * Convert a unigram into its vocabulary id.
     * @param token The unigram to look up
     * @return the vocabulary id for this token
     */
    term_id index(const std::string& token) const;

    /**
     * @return the vocabulary id for the <unk> token
     */
    term_id unk() const;

    /**
     * Returns the score according to the language model for generating
     * the next token given the current state in_state. The context needed
     * for scoring the next word is written to out_state.
     *
     * @param in_state The context, which is either just <s> or was
     * filled for you by a previous call to score()
     * @param token The next token to score (as a word index)
     * @param out_state Storage to write the state for the next query to
     *
     * @return \f$p(w_n \mid w_1, \ldots, w_{n-1})\f$
     */
    float score(const lm_state& in_state, term_id token,
                lm_state& out_state) const;

  private:
    /**
     * Reads precomputed LM data into this object.
     * @param arpa_file The path to the ARPA-formatted file
     */
    void read_arpa_format(const std::string& arpa_file);

    /**
     * Internal log_prob that takes a token_list
     */
    float log_prob(const token_list& tokens) const;

    /**
     * Loads unigram vocabulary from text file
     */
    void load_vocab();

    uint64_t N_; /// The "n" value for this n-gram language model

    std::vector<static_probe_map> lm_;

    std::unordered_map<std::string, term_id> vocabulary_;

    std::string prefix_;

    term_id unk_id_;

    lm_node unk_node_;
};
}
}

#endif
