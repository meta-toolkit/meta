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
        virtual void tokenize(const string & filename, Document & document, unordered_map<string, size_t>* docFreq) const = 0;
};

#endif
