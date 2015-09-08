/**
 * @file sentence.h
 * @author Sean Massung
 */

#ifndef META_SENTENCE_H_
#define META_SENTENCE_H_

#include <stdexcept>
#include <deque>
#include <vector>
#include <string>
#include "util/hash.h"

namespace meta
{
namespace lm
{
/**
 * A sequence of tokens that represents a sentence. Tokens are stored in a list
 * format to enable operations such as insert, substitute, and remove. If an
 * edit is performed, it is remembered as part of an ordered sequence of
 * operations. Further, different weights may be assigned to any arbitrary edit
 * operation, and these weights may also be returned as an ordered sequence.
 * @see Useful in conjunction with lm::diff
 */
class sentence
{
  public:
    using iterator = std::deque<std::string>::iterator;
    using const_iterator = std::deque<std::string>::const_iterator;
    using size_type = std::deque<std::string>::size_type;

    /**
     * Default constructor; an empty sentence.
     */
    sentence() = default;

    /**
     * Creates a sentence based on a text string, parsed with the default filter
     * chain.
     * @param text
     * @param tokenize Whether or not to tokenize the input sentence
     */
    sentence(const std::string& text, bool tokenize = true);

    /**
     * @return a string representation of this sentence
     */
    std::string to_string() const;

    /**
     * @param idx
     * @return the token at the specified index
     */
    const std::string& operator[](size_type idx) const;

    /**
     * Slicing/substring operator
     * @param from index of left side of sentence
     * @param to index of right side of sentence
     */
    sentence operator()(size_type from, size_type to) const;

    /**
     * Replace the token at the specified index with the provided token
     * @param idx
     * @param token
     * @param weight The weight that this edit carries
     */
    void substitute(size_type idx, const std::string& token,
                    double weight = 0.0);

    /**
     * @param idx Index of the token to remove from this sentence
     * @param weight The weight that this edit carries
     */
    void remove(size_type idx, double weight = 0.0);

    /**
     * @param idx Index to insert a token in front of (to insert at beginning,
     * idx = 0)
     * @param token
     * @param weight The weight that this edit carries
     */
    void insert(size_type idx, const std::string& token, double weight = 0.0);

    /**
     * @return the average weight of edits to this sentence
     */
    double average_weight() const;

    /**
     * @return the sequence of edit weights to this sentence
     * @see useful in conjunction with lm::diff
     */
    std::vector<double> weights() const;

    /**
     * @return the string representations of the operations (edits) performed on
     * this sentence
     */
    const std::vector<std::string>& operations() const;

    /**
     * @return the sequence of tokens that comprise this sentence
     */
    const std::deque<std::string>& tokens() const;

    /**
     * @return the token at the front of the sentence
     */
    const std::string& front() const;

    /**
     * @return the token at the end of the sentence
     */
    const std::string& back() const;

    /**
     * Inserts a token at the beginning of the sentence
     * @param token The token to insert
     */
    void push_front(const std::string& token);

    /**
     * Remove the token at the beginning of the sentence
     */
    void pop_front();

    /**
     * Inserts a token at the end of the sentence
     * @param token The token to insert
     */
    void push_back(const std::string& token);

    /**
     * Remove the token at the end of the sentence
     */
    void pop_back();

    /**
     * Emplaces a token at the beginning of the sentence
     */
    template <class... Args>
    void emplace_front(Args&&... args);

    /**
     * Emplaces a token at the end of the sentence
     */
    template <class... Args>
    void emplace_back(Args&&... args);

    /**
     * @return an iterator to the beginning of the sequence
     */
    iterator begin();

    /**
     * @return an iterator to the end of the sequence
     */
    iterator end();

    /**
     * @return a const_iterator to the beginning of the sequence
     */
    const_iterator begin() const;

    /**
     * @return a const_iterator to the end of the sequence
     */
    const_iterator end() const;

    /**
     * @return the number of observations in the sequence
     */
    size_type size() const;

  private:
    /// The tokens (words) in the sentence
    std::deque<std::string> tokens_;

    /// String representations of the sequence of edit oeprations performed
    std::vector<std::string> ops_;

    /// Ordered sequence of edit weights
    std::vector<double> weights_;
};

class sentence_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

inline bool operator==(const sentence& lhs, const sentence& rhs)
{
    return lhs.tokens() == rhs.tokens();
}

inline bool operator!=(const sentence& lhs, const sentence& rhs)
{
    return !(lhs == rhs);
}
}
}

namespace std
{
template <>
struct hash<meta::lm::sentence>
{
#if META_HAS_NONEMPTY_HASH_SUPPORT
    meta::util::murmur_hash<> hasher;
#endif

    size_t operator()(const meta::lm::sentence& sent) const noexcept
    {
#ifndef META_HAS_NONEMPTY_HASH_SUPPORT
        meta::util::murmur_hash<> hasher{89122527};
#endif
        // create a vector of hashes of all the tokens in the sentence
        std::vector<std::size_t> hashed;
        for (const auto& word : sent)
            hashed.push_back(hasher(
                reinterpret_cast<const uint8_t*>(word.data()), word.size()));

        // hash the hashes as sequences of uint8_ts
        return hasher(reinterpret_cast<const uint8_t*>(hashed.data()),
                      hashed.size() * sizeof(std::size_t));
    }
};
}

#endif
