/**
 * @file lm_test.cpp
 * @author Sean Massung
 */

#include "lm/sentence.h"
#include "lm/language_model.h"
#include "test/lm_test.h"
#include "test/inverted_index_test.h"

namespace meta
{
namespace testing
{

int lm_tests()
{
    int num_failed = 0;
    create_config("line");

    num_failed += testing::run_test(
        "lm-test", [&]()
        {
            lm::language_model model{cpptoml::parse_file("config.toml")};
            lm::sentence s1{
                "<s> I disagree with this statement for several reasons . </s>",
                false};
            lm::sentence s2{
                "<s> I disagree with this octopus for several reasons . </s>",
                false};
            lm::sentence s3{"<s> Hello world ! </s>", false};
            lm::sentence s4{"<s> xyz xyz xyz </s>", false};

            ASSERT_APPROX_EQUAL(model.log_prob(s1), -5.0682507);
            ASSERT_APPROX_EQUAL(model.log_prob(s2), -11.7275571);
            ASSERT_APPROX_EQUAL(model.log_prob(s3), -11.0764951);
            ASSERT_APPROX_EQUAL(model.log_prob(s4), -16.4180412);
        });

    return num_failed;
}
}
}
