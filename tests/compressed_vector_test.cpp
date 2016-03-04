/**
 * @file compressed_vector_test.cpp
 * @author Chase Geigle
 */

#include <fstream>

#include "bandit/bandit.h"
#include "meta/io/filesystem.h"
#include "meta/util/random.h"
#include "meta/succinct/compressed_vector.h"

using namespace bandit;

go_bandit([]() {
    using namespace meta;
    using namespace succinct;

    describe("[compressed vector]", []() {
        std::mt19937 rng{47};
        std::vector<uint64_t> values(1000000);
        std::generate(values.begin(), values.end(), [&]() {
            return random::bounded_rand(rng, 65537);
        });

        filesystem::remove_all("compressed-vector-unit-test");

        succinct::make_compressed_vector("compressed-vector-unit-test",
                                         values.begin(), values.end());

        compressed_vector cv{"compressed-vector-unit-test"};
        it("should report the correct size", [&]() {
            AssertThat(cv.size(), Equals(values.size()));
        });

        it("should retrieve correct values", [&]() {
            for (std::size_t i = 0; i < values.size(); ++i)
                AssertThat(cv[i], Equals(values[i]));
        });
    });
});
