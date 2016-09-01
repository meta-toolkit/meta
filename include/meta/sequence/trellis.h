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

#include <unordered_map>
#include <vector>

#include "meta/config.h"
#include "meta/meta.h"
#include "meta/sequence/observation.h"
#include "meta/util/dense_matrix.h"

namespace meta
{
namespace sequence
{

/**
 * Basic trellis for holding score data for the forward/backward algorithm.
 */
class trellis
{
  protected:
    /// storage for the scores
    util::dense_matrix<double> trellis_;

  public:
    /**
     * Constructs a new trellis with the given number of time steps and
     * labels
     *
     * @param size The number of time steps
     * @param labels The number of labels associated with each time step
     */
    trellis(uint64_t size, uint64_t labels);

    /**
     * @return The number of time steps in the trellis.
     */
    uint64_t size() const;

    /**
     * Sets the value in the trellis for the given time step and label.
     *
     * @param idx The time step
     * @param tag The label
     * @param prob The value to be set
     */
    void probability(uint64_t idx, const label_id& tag, double prob);

    /**
     * Obtains the value in the trellis for the given time step and label.
     *
     * @param idx The time step
     * @param tag The label
     * @return the value in the trellis at that location
     */
    double probability(uint64_t idx, const label_id& tag) const;
};

/**
 * Special trellis for the normalized forward algorithm. In addition to
 * storing the scores like a normal trellis, it also stores the normalizers
 * (and can perform normalization on a given time step).
 */
class forward_trellis : public trellis
{
  private:
    /// storage for the normalizers for each time step
    std::vector<double> normalizers_;

  public:
    /**
     * Constructs a forward_trellis with the given number of time steps
     * and labels.
     *
     * @param size The number of time steps
     * @param labels The number of labels
     */
    forward_trellis(uint64_t size, uint64_t labels);

    /**
     * @param idx The time step
     * @return the normalizer used for the given time step
     */
    double normalizer(uint64_t idx) const;

    /**
     * Performs normalization on the given time step and stores the
     * normalizer.
     * @param idx The time step
     */
    void normalize(uint64_t idx);
};

/**
 * Special trellis for the Viterbi algorithm. In addition to storing the
 * scores like a normal trellis, it also can store back pointers indicating
 * the best possible path for each node at each time step.
 */
class viterbi_trellis : public trellis
{
  private:
    /// storage for the back pointers
    util::dense_matrix<label_id> paths_;

  public:
    /**
     * Constructs a viterbi_trellis with the given number of time steps and
     * labels.
     *
     * @param size The number of time steps
     * @param labels The number of labels
     */
    viterbi_trellis(uint64_t size, uint64_t labels);

    /**
     * Sets the back pointer for the given time step and label to the
     * given label.
     *
     * @param idx The time step
     * @param current The current label
     * @param previous The best previous tag for the current label
     */
    void previous_tag(uint64_t idx, const label_id& current,
                      const label_id& previous);

    /**
     * @param idx The time step
     * @param current The label
     * @return the back pointer for the given time step and label.
     */
    const label_id& previous_tag(uint64_t idx, const label_id& current) const;
};
}
}
#endif
