/**
 * @file corpus_test.cpp
 * @author Chase Geigle
 */

#include "bandit/bandit.h"
#include "create_config.h"
#include "meta/corpus/all.h"
#include "meta/corpus/corpus_factory.h"

using namespace bandit;
using namespace snowhouse;
using namespace meta;

void missing_files(const std::string& type) {
    auto config = tests::create_config(type);

    auto prev_prefix = *config->get_as<std::string>("prefix");
    config->insert("prefix", prev_prefix + "/tests");
    config->insert("dataset", "missing-files");

    AssertThrows(corpus::corpus_exception, [&]() {
        auto corp = corpus::make_corpus(*config);
        while (corp->has_next())
            corp->next();
    }());
}

void early_stop(const std::string& type) {
    auto config = tests::create_config(type);

    auto prev_prefix = *config->get_as<std::string>("prefix");
    config->insert("prefix", prev_prefix + "/tests");
    config->insert("dataset", "early-stop");

    auto corp = corpus::make_corpus(*config);
    AssertThrows(corpus::corpus_exception, [&]() {
        while (corp->has_next())
            corp->next();
    }());
}

go_bandit([]() {
    describe("[line-corpus]", []() {
        it("should throw exception on missing files",
           []() { missing_files("line"); });

        it("should throw exception when file ends prematurely",
           []() { early_stop("line"); });
    });

    describe("[gz-corpus]", []() {
        it("should throw exception on missing files",
           []() { missing_files("gz"); });

        it("should throw exception when file ends prematurely",
           []() { early_stop("gz"); });
    });

    describe("[file-corpus]", []() {
        it("should throw exception on missing corpus list",
           []() { missing_files("file"); });

        it("should throw exception on missing document files",
           []() { early_stop("file"); });
    });
});
