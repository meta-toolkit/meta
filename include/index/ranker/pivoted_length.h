/**
 * @file pivoted_length.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PIVOTED_LENGTH_H_
#define META_PIVOTED_LENGTH_H_

#include "index/ranker/ranker.h"
#include "index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * The pivoted document length normalization ranking function
 * @see Amit Singal, Chris Buckley, and Mandar Mitra. Pivoted document length
 * normalization. SIGIR '96, pages 21-29.
 */
class pivoted_length : public ranker
{
  public:
    /**
     * The identifier for this ranker.
     */
    const static std::string id;

    const static constexpr double default_s = 0.20;

    /**
     * @param s
     */
    pivoted_length(double s = default_s);

    /**
     * @param sd
     */
    double score_one(const score_data& sd) override;

  private:
    const double s_;
};

/**
 * Specialization of the factory method used to create pivoted_length
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<pivoted_length>(const cpptoml::toml_group&);
}
}
#endif
