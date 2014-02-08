/**
 * @file tokenizer_test.h
 * @author Sean Massung
 */

#ifndef _TOKENIZER_TEST_H_
#define _TOKENIZER_TEST_H_

#include <string>
#include "tokenizers/all.h"

namespace meta
{
namespace testing
{

#if 0
    template <class Tokenizer>
    void check_tokenizer_expected(Tokenizer & tok, corpus::document doc,
                                  uint64_t num_unique, uint64_t length)
    {
        tok.tokenize(doc);
        ASSERT(doc.frequencies().size() == num_unique);
        ASSERT(doc.length() == length);
        ASSERT(doc.id() == 47);
        if(doc.contains_content())
        {
            ASSERT(doc.path() == "/home/person/filename.txt");
            ASSERT(doc.name() == "filename.txt");
        }
        else
        {
            ASSERT(doc.path() == "../data/sample-document.txt");
            ASSERT(doc.name() == "sample-document.txt");
        }
    }

    void content_tokenize()
    {
        corpus::document doc{"/home/person/filename.txt", doc_id{47}};

        // "one" is a stopword
        std::string content = "one one two two two three four one five";
        doc.set_content(content);

        testing::run_test("content-unigram-word-tokenizer", [&](){
            tokenizers::ngram_word_tokenizer tok{1};
            check_tokenizer_expected(tok, doc, 4, 6);
        });

        testing::run_test("content-bigram-word-tokenizer", [&](){
            tokenizers::ngram_word_tokenizer tok{2};
            check_tokenizer_expected(tok, doc, 4, 5);
        });

        testing::run_test("content-trigram-word-tokenizer", [&](){
            tokenizers::ngram_word_tokenizer tok{3};
            check_tokenizer_expected(tok, doc, 4, 4);
        });
    }

    void file_tokenize()
    {
        corpus::document doc{"../data/sample-document.txt", doc_id{47}};

        testing::run_test("file-unigram-word-tokenizer", [&](){
            tokenizers::ngram_word_tokenizer tok{1};
            check_tokenizer_expected(tok, doc, 93, 142);
        });

        testing::run_test("file-bigram-word-tokenizer", [&](){
            tokenizers::ngram_word_tokenizer tok{2};
            check_tokenizer_expected(tok, doc, 128, 141);
        });

        testing::run_test("file-trigram-word-tokenizer", [&](){
            tokenizers::ngram_word_tokenizer tok{3};
            check_tokenizer_expected(tok, doc, 136, 140);
        });
    }

    void tokenizer_tests()
    {
        content_tokenize();
        file_tokenize();
    }
#endif

int tokenizer_tests()
{
    int num_failed = 0;

    // "one" is a stopword
    std::string content = "one one two two two three four one five";
    num_failed += testing::run_test("tokenizer", [&]()
    {
        tokenizers::ngram_word_tokenizer tok{1};
        corpus::document doc{"none", doc_id{0}};
        doc.set_content(content);
        tok.tokenize(doc);
        ASSERT(doc.length() == 6);
    });

    testing::report(num_failed);
    return num_failed;
}
}
}

#endif
