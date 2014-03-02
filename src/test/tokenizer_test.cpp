/**
 * @file tokenizer_test.cpp
 * @author Sean Massung
 */

#include "test/tokenizer_test.h"

namespace meta
{
namespace testing
{
template <class Tokenizer>
void check_tokenizer_expected(Tokenizer& tok, corpus::document doc,
                              uint64_t num_unique, uint64_t length)
{
    tok.tokenize(doc);
    ASSERT_EQUAL(doc.counts().size(), num_unique);
    ASSERT_EQUAL(doc.length(), length);
    ASSERT_EQUAL(doc.id(), 47);
    if (doc.contains_content())
    {
        ASSERT_EQUAL(doc.path(), "/home/person/filename.txt");
        ASSERT_EQUAL(doc.name(), "filename.txt");
    }
    else
    {
        ASSERT_EQUAL(doc.path(), "../data/sample-document.txt");
        ASSERT_EQUAL(doc.name(), "sample-document.txt");
    }
}

int content_tokenize()
{
    corpus::document doc{"/home/person/filename.txt", doc_id{47}};

    // "one" is a stopword
    std::string content = "one one two two two three four one five";
    doc.set_content(content);
    int num_failed = 0;

    num_failed += testing::run_test("content-unigram-word-tokenizer", [&]()
    {
        tokenizers::ngram_word_tokenizer tok{1};
        check_tokenizer_expected(tok, doc, 4, 6);
    });

    num_failed += testing::run_test("content-bigram-word-tokenizer", [&]()
    {
        tokenizers::ngram_word_tokenizer tok{2};
        check_tokenizer_expected(tok, doc, 4, 5);
    });

    num_failed += testing::run_test("content-trigram-word-tokenizer", [&]()
    {
        tokenizers::ngram_word_tokenizer tok{3};
        check_tokenizer_expected(tok, doc, 4, 4);
    });

    return num_failed;
}

int file_tokenize()
{
    int num_failed = 0;
    corpus::document doc{"../data/sample-document.txt", doc_id{47}};

    num_failed += testing::run_test("file-unigram-word-tokenizer", [&]()
    {
        tokenizers::ngram_word_tokenizer tok{1};
        check_tokenizer_expected(tok, doc, 93, 142);
    });

    num_failed += testing::run_test("file-bigram-word-tokenizer", [&]()
    {
        tokenizers::ngram_word_tokenizer tok{2};
        check_tokenizer_expected(tok, doc, 128, 141);
    });

    num_failed += testing::run_test("file-trigram-word-tokenizer", [&]()
    {
        tokenizers::ngram_word_tokenizer tok{3};
        check_tokenizer_expected(tok, doc, 136, 140);
    });

    return num_failed;
}

int tokenizer_tests()
{
    int num_failed = 0;

    num_failed += content_tokenize();
    num_failed += file_tokenize();

    return num_failed;
}
}
}
