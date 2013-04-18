/**
 * @file ngram_tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NGRAM_TOKENIZER_H_
#define _NGRAM_TOKENIZER_H_

#include <string>
#include <memory>
#include <deque>
#include <unordered_map>

#include "tokenizers/tokenizer.h"
#include "index/document.h"

namespace meta {
namespace tokenizers {

/**
 * Tokenizes documents based on an ngram word model, where the value for n is
 * supplied by the user.  This class is abstract, as it only provides the
 * framework for ngram tokenization.
 */
class ngram_tokenizer: public tokenizer
{
    public:

        /**
         * Constructor.
         * @param n The value of n in ngram.
         */
        ngram_tokenizer(size_t n);

        /**
         * @return the value of n used for the ngrams
         */
        virtual size_t n_value() const;

    protected:

        /**
         * Turns a list of words into an ngram string.
         * @param words - the deque representing a list of words
         * @return the ngrams in string format
         */
        virtual std::string wordify(const std::deque<std::string> & words) const;

    private:

        /** The value of n for this ngram tokenizer */
        size_t _n_val;

};

}
}

#endif
