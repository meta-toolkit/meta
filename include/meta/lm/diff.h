/**
 * @file diff.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LM_DIFF_H_
#define META_LM_DIFF_H_

#include <unordered_map>
#include <unordered_set>

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/hashing/hash.h"
#include "meta/lm/language_model.h"

namespace meta
{
namespace lm
{
/**
 * Uses a language model to transform sentences given a reference text
 * collection. These transformations can be used directly or can be employed as
 * features to represent text data in a wide variety of text mining
 * applications.
 * @see Sean Massung and ChengXiang Zhai. 2015. "SyntacticDiff: Operator-Based
 * Transformation for Comparative Text Mining"
 * @see http://web.engr.illinois.edu/~massung1/files/bigdata-2015.pdf
 * @note It is *very important* that the language model .arpa file and the input
 * to lm::diff are tokenized in the same way!
 *
 * Required config parameters:
 * ~~~toml
 * [diff]
 * n-value = 3 # e.g.
 * max-edits = 2 # e.g., probably something in [1,5]
 * function-words = "path-to-file.txt" # words that may be inserted into
 *                                     # the sentence
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [diff]
 * base-penalty = 0.1
 * insert-penalty = 0.1
 * substitute-penalty = 0.1
 * remove-penalty = 0.1
 * max-candidates = 20
 * lambda = 0.5 # balances scoring between perplexity and edits, in [0,1]
 * lm-generate = false # use LM to insert likely words (may be slow!)
 * ~~~
 */
class diff
{
  public:
    /**
     * @param config_file The file containing configuration information
     */
    diff(const cpptoml::table& config);

    /**
     * Default move constructor.
     */
    diff(diff&&) = default;

    /**
     * @param sent The sentence object to inspect
     * @return the index of the last word in the least-likely ngram according to
     * perplexity
     * Runtime is linear in the sentence length, since log_prob is called on
     * each ngram in the sentence.
     */
    uint64_t least_likely_ngram(const sentence& sent) const;

    /**
     * @return the order of the LM used by this diff object
     */
    uint64_t n_val() const
    {
        return n_val_;
    }

    /**
     * @return the language model used by this diff object
     */
    const language_model& lm() const
    {
        return lm_;
    }

    /**
     * @param sent The sentence to transform
     * @param use_lm
     * @return a sorted list of candidate corrections and their scores
     * The runtime depends on the value of parameters set in the config file:
     * - exponential in the maximum number of edits
     * - linear in n and the sentence length
     */
    std::vector<std::pair<sentence, double>> candidates(const sentence& sent,
                                                        bool use_lm = true);

  private:
    /**
     * @param config_file The file containing configuration information
     */
    void set_stems(const cpptoml::table& config);

    /**
     * @param config_file The file containing configuration information
     */
    void set_function_words(const cpptoml::table& config);

    /**
     * @param sent
     * @param candidates
     * @param depth
     */
    template <class PQ>
    void step(const sentence& sent, PQ& candidates, size_t depth);

    /**
     * @param sent
     * @param idx
     * @param candidates
     * @param depth
     */
    template <class PQ>
    void insert(const sentence& sent, size_t idx, PQ& candidates,
                uint64_t depth);

    /**
     * @param sent
     * @param candidates
     * @param depth
     */
    template <class PQ>
    void lm_ops(const sentence& sent, PQ& candidates, uint64_t depth);
    /**
     * @param sent
     * @param idx
     * @param candidates
     * @param depth
     */
    template <class PQ>
    void remove(const sentence& sent, size_t idx, PQ& candidates,
                uint64_t depth);

    /**
     * @param sent
     * @param idx
     * @param candidates
     * @param depth
     */
    template <class PQ>
    void substitute(const sentence& sent, size_t idx, PQ& candidates,
                    uint64_t depth);

    /**
     * @param candidates
     * @param sent
     */
    template <class PQ>
    void add(PQ& candidates, const sentence& sent);

    language_model lm_;

    uint64_t n_val_;
    uint64_t max_edits_;
    double base_penalty_;
    double insert_penalty_;
    double substitute_penalty_;
    double remove_penalty_;

    /// Chooses whether to do edits at a low-probability location in the
    /// sentence determined by / a LM; if false, edits are performed at every
    /// index.
    bool use_lm_;

    /// map of "stem" -> [words that stem to "stem"]
    std::unordered_map<std::string, std::vector<std::string>> stems_;

    /// List of words that can be inserted into the sentence (default is
    /// function words)
    std::vector<std::string> fwords_;

    /// Keeps track of sentences that have already been generated so we don't
    /// perform redundant calculations
    std::unordered_set<lm::sentence, hashing::hash<>> seen_;

    /// How many candidate sentences to store when calling diff::candidates
    uint64_t max_cand_size_;

    /// Balances perplexity and edit weights.
    double lambda_;

    /// Whether to insert likely words based on the language model.
    bool lm_generate_;
};

/**
 * Exception class for diff operations.
 */
class diff_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
}

#endif
