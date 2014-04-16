/**
 * @file trellis.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_TRELLIS_H_
#define META_SEQUENCE_TRELLIS_H_

#include <vector>
#include <unordered_map>

#include "meta.h"
#include "sequence/observation.h"

namespace meta
{
namespace sequence
{

class trellis
{
  protected:
    std::vector<std::vector<double>> trellis_;

  public:
    trellis(uint64_t size, uint64_t labels);

    uint64_t size() const;

    void probability(uint64_t idx, const label_id& tag, double prob);

    double probability(uint64_t idx, const label_id& tag) const;
};

class forward_trellis : public trellis
{
    std::vector<double> normalizers_;

  public:
    forward_trellis(uint64_t size, uint64_t labels);

    double normalizer(uint64_t idx) const;
    void normalize(uint64_t idx, double value);
};

class viterbi_trellis : public trellis
{
    std::vector<std::unordered_map<tag_t, tag_t>> paths_;

  public:
    viterbi_trellis(uint64_t size, uint64_t labels);

    void previous_tag(uint64_t idx, const tag_t& current,
                      const tag_t& previous);

    const tag_t& previous_tag(uint64_t idx, const tag_t& current);
};
}
}
#endif
