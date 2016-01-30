/**
 * @file language_model_test.cpp
 * @author Sean Massung
 */

#include "bandit/bandit.h"
#include "create_config.h"
#include "meta/lm/sentence.h"
#include "meta/lm/language_model.h"

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

    AssertThat(s1.size(), Equals(11ul));
    AssertThat(s2.size(), Equals(11ul));
    AssertThat(s3.size(), Equals(5ul));
    AssertThat(s4.size(), Equals(5ul));

    // log_prob values calculated with KenLM
    const double delta = 0.0000001;
    AssertThat(model.log_prob(s1), EqualsWithDelta(-5.0682507, delta));
    AssertThat(model.log_prob(s2), EqualsWithDelta(-11.7275571, delta));
    AssertThat(model.log_prob(s3), EqualsWithDelta(-11.07649517, delta));
    AssertThat(model.log_prob(s4), EqualsWithDelta(-16.41804123, delta));

    AssertThat(model.perplexity(s1), EqualsWithDelta(2.88901686, delta));
    AssertThat(model.perplexity(s2), EqualsWithDelta(11.64505672, delta));
    AssertThat(model.perplexity(s3), EqualsWithDelta(164.17201232, delta));
    AssertThat(model.perplexity(s4), EqualsWithDelta(1921.35754394, delta));

    AssertThat(model.perplexity_per_word(s1),
               EqualsWithDelta(model.perplexity(s1) / s1.size(), delta));
    AssertThat(model.perplexity_per_word(s2),
               EqualsWithDelta(model.perplexity(s2) / s2.size(), delta));
    AssertThat(model.perplexity_per_word(s3),
               EqualsWithDelta(model.perplexity(s3) / s3.size(), delta));
    AssertThat(model.perplexity_per_word(s4),
               EqualsWithDelta(model.perplexity(s4) / s4.size(), delta));
}
}

go_bandit([]() {
    auto line_cfg = tests::create_config("line");

    describe("[language-model] sentence", [&]() {

        it("should tokenize strings if requested (true by default)", [&]() {
            std::string orig = "Hello, there (hi).";
            std::string tokenized = "Hello , there ( hi ) .";
            lm::sentence sent1{orig};
            lm::sentence sent2{orig, true};
            lm::sentence sent3{orig, false};
            AssertThat(sent1.to_string(), Equals(tokenized));
            AssertThat(sent2.to_string(), Equals(tokenized));
            AssertThat(sent3.to_string(), Equals(orig));
            AssertThat(sent1.size(), Equals(7ul));
            AssertThat(sent2.size(), Equals(7ul));
            AssertThat(sent3.size(), Equals(3ul));
        });

        it("should support traversal and token extraction", [&]() {
            lm::sentence sent{"Ab cd efg hi j (k)."};
            const uint64_t size = 9;
            AssertThat(sent.size(), Equals(size));

            lm::sentence built;
            AssertThat(built.size(), Equals(0ul));

            uint64_t count = 0;
            for (auto& word : sent) {
                built.push_front(sent[sent.size() - count - 1]);
                AssertThat(sent[count], Equals(word));
                ++count;
            }
            AssertThat(count, Equals(size));
            AssertThat(built, Equals(sent));

            while (built.size() > 0)
                built.pop_back();

            count = 0;
            for (const auto& word : sent) {
                built.push_back(sent[count]);
                ++count;
                AssertThat(built.back(), Equals(word));
            }
            AssertThat(count, Equals(size));
            AssertThat(built, Equals(sent));

            while (built.size() > 0)
                built.pop_front();
            AssertThat(built.size(), Equals(0ul));
            AssertThat(built.to_string(), Equals(""));

            count = 0;
            std::vector<std::string> tokens;
            for (uint64_t i = 0; i < sent.size(); ++i) {
                ++count;
                built.push_back(sent[i]);
                tokens.push_back(sent[i]);
            }
            AssertThat(count, Equals(size));
            AssertThat(built, Equals(sent));
            AssertThat(sent.tokens(), Equals(tokens));
        });
    });

    describe("[language-model] language_model", [&]() {

        it("should create new binary files with correct output",
           [&]() { run_test(*line_cfg); });

        it("should read binary files with correct output",
           [&]() { run_test(*line_cfg); });

        filesystem::delete_file("test-lm-0.binlm");
        filesystem::delete_file("test-lm-1.binlm");
        filesystem::delete_file("test-lm-2.binlm");
        filesystem::delete_file("test-lm-0.strings");
    });
});
