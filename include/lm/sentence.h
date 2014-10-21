/**
 * @file sentence.h
 * @author Sean Massung
 */

#ifndef META_SENTENCE_H_
#define META_SENTENCE_H_

#include <deque>
#include <vector>
#include <string>

namespace meta
{
namespace lm
{
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
     */
    sentence(const std::string& text);

    /**
     * @return a string representation of this sentence
     */
    std::string to_string() const;

    /**
     * @param idx
     * @return the token at the specified index
     */
    const std::string& operator[](size_type idx) const;

    sentence operator()(size_type from, size_type to) const;

    /**
     * @param idx
     * @param token
     * @return replace the token at the specified index with the provided token
     */
    void substitute(size_type idx, const std::string& token);

    /**
     * @param idx Index of the token to remove from this sentence
     */
    void remove(size_type idx);

    /**
     * @param idx Index to insert a token in front of (to insert at beginning,
     * idx = 0)
     * @param token
     */
    void insert(size_type idx, const std::string& token);

    const std::vector<std::string>& operations() const;

    std::string front() const;

    std::string back() const;

    void push_front(const std::string& token);

    void pop_front();

    void push_back(const std::string& token);

    void pop_back();

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
    std::deque<std::string> tokens_;
    std::vector<std::string> ops_;
};

class sentence_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
}

#endif
