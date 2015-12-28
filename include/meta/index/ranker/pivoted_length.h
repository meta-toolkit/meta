/**
 * @file pivoted_length.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PIVOTED_LENGTH_H_
#define META_PIVOTED_LENGTH_H_

#include "meta/index/ranker/ranker.h"
#include "meta/index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * The pivoted document length normalization ranking function
 * @see Amit Singal, Chris Buckley, and Mandar Mitra. Pivoted document length
 * normalization. SIGIR '96, pages 21-29.
 *
 * Required config parameters:
 * ~~~toml
 * [ranker]
 * method = "pivoted-length"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * s = 0.2
 * ~~~
 */
class pivoted_length : public ranker
{
  public:
    /// Identifier for this ranker.
    const static util::string_view id;

    /// Default value of s parameter
    const static constexpr float default_s = 0.2f;

    /**
     * @param s
     */
    pivoted_length(float s = default_s);

    /**
     * Loads a pivoted_length ranker from a stream.
     * @param in The stream to read from
     */
    pivoted_length(std::istream& in);

    /**
     * @param sd the score_data for this query
     */
    float score_one(const score_data& sd) override;

    void save(std::ostream& out) const override;

  private:
    /// s parameter for pivoted_length normalization
    const float s_;
};

/**
 * Specialization of the factory method used to create pivoted_length
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<pivoted_length>(const cpptoml::table&);
}
}
#endif
