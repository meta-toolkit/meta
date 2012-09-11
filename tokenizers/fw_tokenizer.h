/**
 * @file fw_tokenizer.h
 */

#ifndef _FW_TOKENIZER_H_
#define _FW_TOKENIZER_H_

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include "tokenizer.h"

class Document;

/**
 * Tokenizes documents based on the author's usage of function words.
 */
class FWTokenizer : public Tokenizer
{
    public:

        /**
         * Constructor; initializes the tokenizer with a set function word list.
         */
        FWTokenizer(const std::string & fwFile);

        /**
         * Tokenizes a file into a Document.
         * @param document - the Document to store the tokenized information in
         * @param docFreq - optional parameter to store IDF values in
         */
        virtual void tokenize(Document & document,
                std::shared_ptr<std::unordered_map<TermID, unsigned int>> docFreq);

    private:

        /** a list of function words to be used by the tokenizer */
        std::unordered_set<std::string> _functionWords;
};

#endif
