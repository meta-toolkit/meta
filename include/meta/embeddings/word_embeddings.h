/**
 * @file word_embeddings.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_EMBEDDINGS_WORD_EMBEDDINGS_H_
#define META_EMBEDDINGS_WORD_EMBEDDINGS_H_

#include <istream>
#include <stdexcept>

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/hashing/probe_map.h"
#include "meta/util/aligned_allocator.h"
#include "meta/util/array_view.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace embeddings
{

struct embedding
{
    std::size_t tid;
    util::array_view<const double> v;
};

struct scored_embedding
{
    embedding e;
    double score;
};

/**
 * A read-only model for accessing word embeddings.
 */
class word_embeddings
{
  public:
    /**
     * Loads word embeddings from files.
     *
     * @param vocab The stream to read the vocabulary from
     * @param vectors The stream to read the vectors from
     */
    word_embeddings(std::istream& vocab, std::istream& vectors);

    /**
     * Loads word embeddings from parallel streams. The word embeddings
     * will be component-wise sums of the word embeddings from the two
     * streams.
     *
     * @param vocab The stream to read the vocabulary from
     * @param first The first stream
     * @param second The second stream
     */
    word_embeddings(std::istream& vocab, std::istream& first,
                    std::istream& second);

    /**
     * @param term The term to look up
     * @return the embedding vector (as an array_view) for the given term,
     *  or the vector for the unknown word as appropriate
     */
    embedding at(util::string_view term) const;

    /**
     * @param tid The term id to look up
     * @return the term (as a string_view) represented by that term id
     */
    util::string_view term(std::size_t tid) const;

    /**
     * @param query A vector of the same length as a word embedding to
     *  query for
     * @param k The number of embeddings to return
     * @return the top k word scored_embeddings closest to the query
     */
    std::vector<scored_embedding> top_k(util::array_view<const double> query,
                                        std::size_t k = 100) const;

  private:
    util::array_view<double> vector(std::size_t tid);

    util::array_view<const double> vector(std::size_t tid) const;

    void load_vocab(std::istream& vocab);

    /// The size of the word embeddings
    const std::size_t vector_size_;

    /// A list of all of the strings in the vocabulary, indexed by id
    util::aligned_vector<std::string> id_to_term_;

    /// A hash table from a term to its id
    hashing::probe_map<util::string_view, std::size_t> term_to_id_;

    /// The embeddings matrix
    util::aligned_vector<double> embeddings_;
};

/**
 * Exception thrown when interacting with the word_embeddings model.
 */
class word_embeddings_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

word_embeddings load_embeddings(const cpptoml::table& config);
}
}
#endif
