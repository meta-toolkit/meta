/**
 * @file forward_index_test.cpp
 * @author Sean Massung
 */

#include "test/forward_index_test.h"

namespace meta
{
namespace testing
{

void create_libsvm_config()
{
    auto orig_config = cpptoml::parse_file("config.toml");
    std::string config_filename{"test-config.toml"};
    std::ofstream config_file{config_filename};
    config_file << "prefix = \"" << *orig_config.get_as<std::string>("prefix")
                << "\"\n"
                << "corpus-type = \"line-corpus\"\n"
                << "dataset = \"breast-cancer\"\n"
                << "forward-index = \"bcancer-fwd\"\n"
                << "inverted-index = \"bcancer-inv\"\n"
                << "[[analyzers]]\n"
                << "method = \"libsvm\"\n";
}

template <class Index>
void check_bcancer_expected(Index& idx)
{
    ASSERT_EQUAL(idx.num_docs(), 683ul);
    ASSERT_EQUAL(idx.unique_terms(), 10ul);

    std::ifstream in{"../data/bcancer-metadata.txt"};
    doc_id id{0};
    uint64_t size;
    while (in >> size)
    {
        ASSERT_EQUAL(idx.doc_size(doc_id{id}), size);
        ++id;
    }
    ASSERT_EQUAL(id, idx.num_docs());
}

template <class Index>
void check_ceeaus_expected_fwd(Index& idx)
{
    ASSERT_EQUAL(idx.num_docs(), 1008ul);
    ASSERT_EQUAL(idx.unique_terms(), 3944ul);

    std::ifstream in{"../data/ceeaus-metadata.txt"};
    uint64_t size;
    uint64_t unique;
    doc_id id{0};
    while (in >> size >> unique)
    {
        // don't care about unique terms per doc in forward_index (yet)
        ASSERT_EQUAL(idx.doc_size(id), size);
        ++id;
    }

    // make sure there's exactly the correct amount
    ASSERT_EQUAL(id, idx.num_docs());
}

template <class Index>
void check_bcancer_doc_id(Index& idx)
{
    doc_id d_id{47};
    term_id first;
    double second;
    std::ifstream in{"../data/bcancer-doc-count.txt"};
    auto pdata = idx.search_primary(d_id);
    for (auto& count : pdata->counts())
    {
        in >> first;
        in >> second;
        ASSERT_EQUAL(first - 1, count.first); // - 1 because libsvm format
        ASSERT_APPROX_EQUAL(second, count.second);
    }
}

template <class Index>
void check_ceeaus_doc_id(Index& idx)
{
    doc_id d_id{47};
    term_id first;
    double second;
    std::ifstream in{"../data/ceeaus-doc-count.txt"};
    auto pdata = idx.search_primary(d_id);
    for (auto& count : pdata->counts())
    {
        in >> first;
        in >> second;
        ASSERT_EQUAL(first, count.first);
        ASSERT_APPROX_EQUAL(second, count.second);
    }
}

void ceeaus_forward_test()
{
    auto idx = index::make_index<index::forward_index, caching::splay_cache>(
        "test-config.toml", uint32_t{10000});
    check_ceeaus_expected_fwd(*idx);
    check_ceeaus_doc_id(*idx);
}

void bcancer_forward_test()
{
    auto idx = index::make_index<index::forward_index, caching::splay_cache>(
        "test-config.toml", uint32_t{10000});
    check_bcancer_expected(*idx);
    check_bcancer_doc_id(*idx);
}

int forward_index_tests()
{
    create_config("file");

    int num_failed = 0;

    num_failed += testing::run_test("forward-index-build-file-corpus", [&]()
    {
        system("rm -rf ceeaus-*");
        ceeaus_forward_test();
    });

    num_failed += testing::run_test("forward-index-read-file-corpus", [&]()
    {
        ceeaus_forward_test();
        system("rm -rf ceeaus-* test-config.toml");
    });

    create_config("line");

    num_failed += testing::run_test("forward-index-build-line-corpus", [&]()
    {
        system("rm -rf ceeaus-*");
        ceeaus_forward_test();
    });

    num_failed += testing::run_test("forward-index-read-line-corpus", [&]()
    {
        ceeaus_forward_test();
        system("rm -rf ceeaus-* test-config.toml");
    });

    create_libsvm_config();

    num_failed += testing::run_test("forward-index-build-libsvm", [&]()
    {
        system("rm -rf bcancer-*");
        bcancer_forward_test();
    });

    num_failed += testing::run_test("forward-index-load-libsvm", [&]()
    {
        bcancer_forward_test();
        system("rm -rf bcancer-* test-config.toml");
    });

    return num_failed;
}
}
}
