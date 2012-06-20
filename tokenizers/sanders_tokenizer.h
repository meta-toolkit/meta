/**
 * @file sanders_tokenizer.h
 */

#ifndef _SANDERS_TOKENIZER_H_
#define _SANDERS_TOKENIZER_H_

#include <string.h>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <iterator>

#include "libstemmer/libstemmer.h"
#include "index/document.h"
#include "ngram_tokenizer.h"

using std::istringstream;
using std::vector;
using std::unordered_map;
using std::unordered_set;

/**
 * Tokenizes documents based on an ngram word model, where the value for n is supplied by the user.
 */
class SandersTokenizer : public NgramTokenizer
{
    public:
        /**
         * Constructor that simply passes arguments to the base class.
         */
        SandersTokenizer(size_t n): NgramTokenizer(n){ /* nothing */ }

        /**
         * Tokenizes a file into a Document.
         * @param filename - the file to read tokens from
         * @param document - the Document to store the tokenized information in
         * @param docFreqs - optional parameter to store IDF values in
         */
        virtual void tokenize(const string & filename, Document & document, unordered_map<TermID, unsigned int>* docFreqs);

};

#endif
