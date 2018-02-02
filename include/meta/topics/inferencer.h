/**
 * @file topics/inferencer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOPICS_INFERENCER_H_
#define META_TOPICS_INFERENCER_H_

#include "meta/config.h"
#include "meta/topics/topic_model.h"
#include "meta/util/progress.h"

namespace meta
{
namespace topics
{

/**
 * A base class for topic model inference methods to be run on unseen (new)
 * documents. This class is useless on its own, but provides a unified
 * interface for loading in topic model output from disk and provides the
 * necessary information for specific inferencers to be built.
 */
class inferencer
{
  public:
    /**
     * Constructs an inferencer by consulting the [lda] configuration
     * group of the supplied config.
     *
     * @param config a cpptoml::table that contains an [lda] subtable.
     */
    inferencer(const cpptoml::table& config);

    /**
     * Constructs an inferencer from an input stream representing the
     * topics file from a topic model (.phi.bin) and the desired
     * (symmetric) Dirichlet prior parameter.
     *
     * @param topic_stream an istream reading the .phi.bin file from a
     * topic model
     * @param alpha the parameter for a symmetric Dirichlet prior for the
     * proportions to be inferred
     */
    inferencer(std::istream& topic_stream, double alpha);

    /**
     * @param k the topic identifier
     * @return the term distribution for the given topic
     */
    const stats::multinomial<term_id>& term_distribution(topic_id k) const;

    /**
     * @return the number of topics
     */
    std::size_t num_topics() const;

    /**
     * @return the Dirichlet prior to use for inferred topic proportions
     * for new documents
     */
    const stats::dirichlet<topic_id>& proportions_prior() const;

  private:
    void load_from_stream(std::istream& topic_stream);

    /// The topics, indexed by topic_id
    std::vector<stats::multinomial<term_id>> topics_;

    /// The prior distribution to use for inferred topic proportions
    stats::dirichlet<topic_id> prior_;
};

class inferencer_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
#endif
