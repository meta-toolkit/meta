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

  private:
    /// The file used to create classifiers and indexes
    std::string config_file_;
};
}
}
}
#endif
