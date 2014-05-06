/**
 * @file window_lda.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOPICS_WINDOW_LDA_H_
#define META_TOPICS_WINDOW_LDA_H_

#include <cstdint>
#include <random>
#include <numeric>
#include <vector>

#include "sequence/sequence.h"
#include "topics/lda_model.h"
#include "util/dense_matrix.h"
#include "util/identifiers.h"
#include "util/invertible_map.h"
#include "util/mapping.h"
#include "util/progress.h"

namespace meta
{
namespace topics
{

MAKE_NUMERIC_IDENTIFIER(segment_id, uint64_t)

/**
 * Window-constrained LDA: a modification of LDA for constraining topic
 * membership probabilities within predefined windows.
 *
 * TODO: add gen model description here
 */
class window_lda
{
  public:
    class dataset
    {
      private:
        using sequence_list = std::vector<sequence::sequence>;

      public:
        void add_sequence(sequence::sequence seq, class_label label)
        {
            sequences_.emplace_back(std::move(seq));
            labels_.emplace_back(label);
        }

        sequence_list::const_iterator begin() const
        {
            return sequences_.cbegin();
        }

        sequence_list::const_iterator end() const
        {
            return sequences_.cend();
        }

        const sequence::sequence& at(uint64_t i) const
        {
            return sequences_.at(i);
        }

        const class_label& label(uint64_t i) const
        {
            return labels_.at(i);
        }

        uint64_t vocab_map(const std::string& str)
        {
            return vmap_(str);
        }

        std::string vocab_map(uint64_t id) const
        {
            return vmap_(id);
        }

        uint64_t size() const
        {
            return sequences_.size();
        }

        uint64_t vocab_size() const
        {
            return vmap_.size();
        }

        uint64_t doc_size(uint64_t idx) const
        {
            const auto& s = at(idx);

            uint64_t sum = 0;
            for (const auto& obs : s)
            {
                for (const auto& p : obs.features())
                    sum += p.second;
            }
            return sum;
        }

        void save_vocabulary(const std::string& filename) const
        {
            vmap_.save(filename);
        }

      private:
        class vocab
        {
          public:
            uint64_t operator()(const std::string& str)
            {
                auto sze = map_.size();
                if (!map_.contains_key(str))
                    map_.insert(str, sze);
                return map_.get_value(str);
            }

            std::string operator()(uint64_t id) const
            {
                return map_.get_key(id);
            }

            uint64_t size() const
            {
                return map_.size();
            }

            void save(const std::string& filename) const
            {
                map::save_mapping(map_, filename);
            }

          private:
            util::invertible_map<std::string, uint64_t> map_;

        } vmap_;

        sequence_list sequences_;
        std::vector<class_label> labels_;
    };

    window_lda(uint64_t num_topics, double alpha, double beta);

    void learn(const dataset& dset, uint64_t burn_in, uint64_t iters,
               double convergence = 1e-6);

    void save(const std::string& prefix, const dataset& dset) const;

  private:
    void initialize(const dataset& dset, printing::progress& progress);

    void perform_iteration(const dataset& dset, printing::progress& progress,
                           uint64_t iter);

    void decrease_counts(topic_id topic, doc_id doc,
                         const sequence::observation& window);

    void increase_counts(topic_id topic, doc_id doc,
                         const sequence::observation& window);

    topic_id sample_topic(doc_id doc, const sequence::observation& window,
                          uint64_t vocab_size);

    double corpus_likelihood(const dataset& dset) const;

    void save_doc_topic_distributions(const std::string& filename) const;
    void save_topic_term_distributions(const std::string& filename,
                                       const dataset& dset) const;
    void save_segments(const std::string& filename, const dataset& dset) const;
    void save_for_lrr(const std::string& filename, const dataset& dset) const;

    /**
     * \f$\sigma\f$, indexed as \f$(j, i)\f$, the number of segments in
     * document \f$j\f$ assigned to topic \f$i\f$.
     */
    util::dense_matrix<uint64_t> sigma_;

    /**
     * \f$\delta\f$, indexed as \f$(r, i)\f$, the number of times word
     * type \f$r\f$ is assigned to topic \f$i\f$ across all documents.
     */
    util::dense_matrix<uint64_t> delta_;

    /**
     * The number of words in the corpus that have been assigned a given
     * topic. Indexed as \f$(i)\f$ where \f$i\f$ is a topic id.
     */
    std::vector<uint64_t> topic_count_;

    /**
     * \f$z\f$, indexed as \f$(j, x)\f$, the topic assignment for the
     * \f$x\f$-th segment in the \f$j\f$-th document.
     */
    std::vector<std::vector<topic_id>> segment_topics_;

    /**
     * The number of topics to find.
     */
    const uint64_t num_topics_;

    /**
     * (Symmetric) Dirichlet hyper-parameter for the prior on
     * \f$\theta\f$, the topic proportions.
     */
    const double alpha_;

    /**
     * (Symmetric) Dirichlet hyper-parameter for the prior on \f$\phi\f$,
     * the topic distributions.
     */
    const double beta_;

    /**
     * Random number generator to be used for sampling.
     */
    std::mt19937_64 rng_;
};


}
}
#endif
