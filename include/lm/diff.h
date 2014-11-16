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
#include "lm/language_model.h"

namespace meta
{
namespace lm
{
class diff
{
  public:
    /**
     * @param config_file The file containing configuration information
     */
    diff(const cpptoml::toml_group& config);

    /**
     * @param sent The sentence to transform
     * @param use_lm
     * @return a sorted list of candidate corrections and their scores
     */
    std::vector<std::pair<sentence, double>> candidates(const sentence& sent,
                                                        bool use_lm = true);

  private:
    /**
     * @param config_file The file containing configuration information
     */
    void set_stems(const cpptoml::toml_group& config);

    /**
     * @param config_file The file containing configuration information
     */
    void set_function_words(const cpptoml::toml_group& config);

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
    std::unordered_map<std::string, std::vector<std::string>> stems_;
    std::vector<std::string> fwords_;
    std::unordered_set<std::string> seen_;
    static constexpr uint64_t max_cand_size_ = 20;
    bool use_lm_;

    /// balance between perplexity and edit weights
    static constexpr double lambda_ = 0.5;
};

class diff_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
}

#endif
