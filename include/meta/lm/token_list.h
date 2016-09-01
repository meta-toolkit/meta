/**
 * @file token_list.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LM_TOKEN_LIST_H_
#define META_LM_TOKEN_LIST_H_

#include <sstream>
#include <unordered_map>

#include "meta/config.h"
#include "meta/hashing/hash.h"
#include "meta/lm/sentence.h"
#include "meta/meta.h"

namespace meta
{
namespace lm
{
class token_list
{
  public:
    /**
     * Constructor that takes a std::string, splits it, and assigns ids to each
     * token based on vocab
     * @param ngram
     * @param vocab
     */
    token_list(const std::string& ngram,
               const std::unordered_map<std::string, term_id>& vocab);

    /**
     * Constructor that takes an lm::sentence and assigns ids to each one based
     * on vocab
     * @param ngram
     * @param vocab
     */
    token_list(const lm::sentence& ngram,
               const std::unordered_map<std::string, term_id>& vocab);

    /**
     * Constructor that creates a token list with a single element
     * @param val
     */
    token_list(term_id val);

    /**
     * Default constructor.
     */
    token_list() = default;

    /**
     * @param idx
     * @return the token id of the element at location idx
     */
    const term_id& operator[](uint64_t idx) const;

    /**
     * @param idx
     * @return the token id of the element at location idx
     */
    term_id& operator[](uint64_t idx);

    /**
     * @return the number of tokens in this list
     */
    uint64_t size() const;

    /**
     * Add elem to the end of the list
     * @param elem
     */
    void push_back(term_id elem);

    /**
     * Remove the first token
     */
    void pop_front();

    /**
     * Remove the last token
     */
    void pop_back();

    /**
     * @return the underlying container of term_id tokens
     */
    const std::vector<term_id>& tokens() const;

  private:
    std::vector<term_id> tokens_;
};

inline bool operator==(const token_list& lhs, const token_list& rhs)
{
    return lhs.tokens() == rhs.tokens();
}

inline bool operator!=(const token_list& lhs, const token_list& rhs)
{
    return !(lhs == rhs);
}

template <class HashAlgorithm>
void hash_append(HashAlgorithm& h, const token_list& list)
{
    h(list.tokens().data(), sizeof(term_id) * list.size());
    hash_append(h, list.size());
}
}
}

#endif
