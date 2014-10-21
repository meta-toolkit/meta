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
    diff(const std::string& config_file,
         uint64_t max_depth = default_max_depth_);

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
    void set_stems(const std::string& config_file);

    /**
     * @param config_file The file containing configuration information
     */
    void set_function_words(const std::string& config_file);

    /**
     * @param sent
     * @param candidates
     * @param depth
     */
    template <class PQ>
    void step(const sentence& sent, PQ& candidates, size_t depth);

    /**
     * @param sent
     * @param candidates
     * @param depth
     */
    template <class PQ>
    void step_lm(const sentence& sent, PQ& candidates, size_t depth);

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
     * @param idx
     * @param candidates
     * @param depth
     * @param substitute
     */
    template <class PQ>
    void lm_ops(const sentence& sent, size_t idx, PQ& candidates,
                uint64_t depth, bool substitute);
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

    language_model lm_;
    std::vector<std::string> fwords_;
    std::unordered_map<std::string, std::vector<std::string>> stems_;
    std::unordered_set<std::string> seen_;
    uint64_t max_depth_;
    static constexpr uint64_t default_max_depth_ = 2;
    static constexpr uint64_t n_val_ = 3;
};
}
}

#endif
