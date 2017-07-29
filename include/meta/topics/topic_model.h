/**
 * @file topic_model.h
 * @author Matt Kelly
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOPICS_TOPICS_H_
#define META_TOPICS_TOPICS_H_

#include <istream>
#include <vector>

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/meta.h"
#include "meta/stats/multinomial.h"
#include "meta/util/fixed_heap.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace topics
{

struct term_prob
{
    term_id tid;
    double probability;
};

struct topic_prob
{
    topic_id tid;
    double probability;
};

/**
 * A read-only model for accessing topic models.
 */
class topic_model
{
  public:
    /**
     * Load topic models from files.
     *
     * @param theta The stream to read the vocabulary from
     * @param phi The stream to read the vectors from
     */
    topic_model(std::istream& theta, std::istream& phi);

    /**
     * @param topic_id The topic to use
     * @param k The number of words to return
     * @return the top k most probable words in the topic
     */
    std::vector<term_prob> top_k(topic_id tid, std::size_t k = 10) const;

    /**
     * @param topic_id The topic to use
     * @param k The number of words to return
     * @param score A scoring function to weight the raw probabilities
     * @return the top k most probable words in the topic
     */
    template <typename T>
    std::vector<term_prob> top_k(topic_id tid, std::size_t k, T&& score) const;

    /**
     * @param doc_id The document we are concerned with
     * @return The probability of each of k topics for the
     * given document
     */
    const stats::multinomial<topic_id>& topic_distribution(doc_id doc) const;

    /**
     * @param k The topic we are concerned with
     * @return The distribution over terms for the specified topic
     */
    const stats::multinomial<term_id>& term_distribution(topic_id k) const;

    /**
     * @param topic_id The topic we are concerned with
     * @param term_id The term we are concerned with
     * @return The probability of the term for the given topic
     */
    double term_probability(topic_id top_id, term_id tid) const;

    /**
     * @param doc The document we are concerned with
     * @param topic_id The topic we are concerned with
     * @return The probability for the given topic
     */
    double topic_probability(doc_id doc, topic_id tid) const;

    /**
     * @return The number of topics
     */
    std::size_t num_topics() const;

    /**
     * @return The number of unique words
     */
    std::size_t num_words() const;

    /**
     * @return The number of documents
     */
    std::size_t num_docs() const;

  private:
    /**
     * The number of topics.
     */
    const std::size_t num_topics_;

    /**
     * The number of total unique words.
     */
    const std::size_t num_words_;

    /**
     * The number of documents.
     */
    const std::size_t num_docs_;

    /**
     * The term probabilities by topic
     */
    std::vector<stats::multinomial<term_id>> topic_term_probabilities_;

    /**
     * The term probabilities by topic
     */
    std::vector<stats::multinomial<topic_id>> doc_topic_probabilities_;
};

template <typename T>
std::vector<term_prob> topic_model::top_k(topic_id tid, std::size_t k,
                                          T&& score) const
{
    auto pairs = util::make_fixed_heap<term_prob>(
        k, [](const term_prob& a, const term_prob& b) {
            return a.probability > b.probability;
        });

    auto current_topic = topic_term_probabilities_[tid];

    for (term_id i{0}; i < num_words_; ++i)
    {
        pairs.push(term_prob{i, score(tid, i)});
    }

    return pairs.extract_top();
}

class topic_model_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

topic_model load_topic_model(const cpptoml::table& config);
}
}

#endif
