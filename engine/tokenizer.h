/**
 * @file tokenizer.h
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <unordered_map>
#include "document.h"
#include "parse_tree.h"

using std::unordered_map;

/**
 * An abstract class that produces tokens.
 */
class Tokenizer
{
    public:
        /**
         *
         */
        virtual unordered_map<string, size_t> getTokens(const string & filename) const = 0;
};

#endif
