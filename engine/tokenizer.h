/**
 * @file tokenizer.h
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "document.h"
#include "parse_tree.h"

/**
 * An abstract class that produces tokens.
 */
class Tokenizer
{
    public:
        /**
         *
         */
        virtual void tokenize(const string & filename, Document & document) const = 0;
};

#endif
