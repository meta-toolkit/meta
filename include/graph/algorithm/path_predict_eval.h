/**
 * @file path_predict_eval.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_PATH_PREDICT_EVAL_H_
#define META_PATH_PREDICT_EVAL_H_

#include <set>
#include "classify/classifier/all.h"
#include "index/forward_index.h"

namespace meta
{
namespace graph
{
namespace algorithm
{
class path_predict_eval
{
  public:
    /**
     * @param config_file The file used to create classifiers and indexes
     */
    path_predict_eval(const std::string& config_file);

    /**
     * Runs the relationship prediction as a classification problem.
     */
    void predictions();

    /**
     * Runs the relationship prediction as a ranking problem.
     */
    void rankings();

    /**
     * @param orig_docs The original dataset
     * @return a vector of evenly-partitioned documents
     */
    template <class Index>
    static std::vector<doc_id> partition(const std::vector<doc_id>& orig_docs,
                                         Index& idx);

  private:
    struct rank_result
    {
        rank_result(const std::string& n, double s, class_label r)
            : name{n}, score{s}, relevance{r}
        {
            // nothing
        }
        bool operator<(const rank_result& other) const
        {
            return other.score < score;
        }
        std::string name;
        double score;
        class_label relevance;
    };

    void eval_ranks();

    std::unordered_map<std::string, std::set<rank_result>> ranks_;

    /// The file used to create classifiers and indexes
    std::string config_file_;
};
}
}
}
#endif
