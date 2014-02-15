/**
 * @file analyzer_test.h
 * @author Sean Massung
 */

#ifndef _TOKENIZER_TEST_H_
#define _TOKENIZER_TEST_H_

#include <string>
#include "analyzers/all.h"

namespace meta
{
namespace testing
{
template <class Analyzer>
void check_analyzer_expected(Analyzer& tok, corpus::document doc,
                              uint64_t num_unique, uint64_t length)
{
    tok.tokenize(doc);
    ASSERT(doc.counts().size() == num_unique);
    ASSERT(doc.length() == length);
    ASSERT(doc.id() == 47);
    if (doc.contains_content())
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

int content_tokenize()
{
    corpus::document doc{"/home/person/filename.txt", doc_id{47}};

    // "one" is a stopword
    std::string content = "one one two two two three four one five";
    doc.set_content(content);
    int num_failed = 0;

    num_failed += testing::run_test("content-unigram-word-analyzer", [&]()
    {
        analyzers::ngram_word_analyzer tok{1};
        check_analyzer_expected(tok, doc, 4, 6);
    });

    num_failed += testing::run_test("content-bigram-word-analyzer", [&]()
    {
        analyzers::ngram_word_analyzer tok{2};
        check_analyzer_expected(tok, doc, 4, 5);
    });

    num_failed += testing::run_test("content-trigram-word-analyzer", [&]()
    {
        analyzers::ngram_word_analyzer tok{3};
        check_analyzer_expected(tok, doc, 4, 4);
    });

    return num_failed;
}

int file_tokenize()
{
    int num_failed = 0;
    corpus::document doc{"../data/sample-document.txt", doc_id{47}};

    num_failed += testing::run_test("file-unigram-word-analyzer", [&]()
    {
        analyzers::ngram_word_analyzer tok{1};
        check_analyzer_expected(tok, doc, 93, 142);
    });

    num_failed += testing::run_test("file-bigram-word-analyzer", [&]()
    {
        analyzers::ngram_word_analyzer tok{2};
        check_analyzer_expected(tok, doc, 128, 141);
    });

    num_failed += testing::run_test("file-trigram-word-analyzer", [&]()
    {
        analyzers::ngram_word_analyzer tok{3};
        check_analyzer_expected(tok, doc, 136, 140);
    });

    return num_failed;
}

int analyzer_tests()
{
    int num_failed = 0;

    num_failed += content_tokenize();
    num_failed += file_tokenize();

    testing::report(num_failed);
    return num_failed;
}
}
}

#endif
