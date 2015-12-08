/**
 * @file language_model_test.cpp
 * @author Sean Massung
 */

#include "bandit/bandit.h"
#include "create_config.h"
#include "lm/sentence.h"
#include "lm/language_model.h"

using namespace bandit;
using namespace meta;

namespace {

void run_test(const cpptoml::table& line_cfg) {
    lm::language_model model{line_cfg};
    lm::sentence s1{
        "<s> I disagree with this statement for several reasons . </s>", false};
    lm::sentence s2{
        "<s> I disagree with this octopus for several reasons . </s>", false};
    lm::sentence s3{"<s> Hello world ! </s>", false};
    lm::sentence s4{"<s> xyz xyz xyz </s>", false};

    const double delta = 0.0000001;
    AssertThat(model.log_prob(s1), EqualsWithDelta(-5.0682507, delta));
    AssertThat(model.log_prob(s2), EqualsWithDelta(-11.7275571, delta));
    AssertThat(model.log_prob(s3), EqualsWithDelta(-11.07649517, delta));
    AssertThat(model.log_prob(s4), EqualsWithDelta(-16.41804123, delta));
};

void delete_lm_files() {
    filesystem::delete_file("test-lm-0.binlm");
    filesystem::delete_file("test-lm-1.binlm");
    filesystem::delete_file("test-lm-2.binlm");
    filesystem::delete_file("test-lm-0.strings");
}
}

go_bandit([]() {
    auto line_cfg = tests::create_config("line");

    describe("[language-model]", [&]() {

        it("should create new binary files with correct output",
           [&]() { run_test(*line_cfg); });

        it("should read binary files with correct output",
           [&]() { run_test(*line_cfg); });

        delete_lm_files();
    });
});
