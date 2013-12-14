/**
 * @file tokenizer_test.h
 */

#ifndef _TOKENIZER_TEST_H_
#define _TOKENIZER_TEST_H_

#include <string>
#include "tokenizers/all.h"

namespace meta {
namespace testing {

    void tokenizer_tests()
    {
        // "one" is a stopword
        std::string content = "one one two two two three four one five";
        testing::run_test("tokenizer", [&](){
            tokenizers::ngram_word_tokenizer tok{1};
            corpus::document doc{"none", doc_id{0}};
            doc.set_content(content);
            tok.tokenize(doc);
            ASSERT(doc.length() == 6);
        });
    }

}
}

#endif
