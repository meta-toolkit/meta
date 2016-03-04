/**
 * @file sarray_test.cpp
 * @author Chase Geigle
 */

#include "bandit/bandit.h"
#include "meta/io/filesystem.h"
#include "meta/succinct/sarray.h"

using namespace bandit;

go_bandit([]() {
    describe("[sarray]", []() {
        using namespace meta;
        using namespace succinct;

        it("should give correct rank results (very sparse)", []() {

            std::vector<uint64_t> positions{100,  200,   222,
                                            1024, 10000, 1331337};
            uint64_t num_bits = 2000000;

            filesystem::remove_all("sarray-unit-test");
            auto storage = make_sarray("sarray-unit-test", positions.begin(),
                                       positions.end(), num_bits);
            sarray_rank ranks{"sarray-unit-test", storage};

            AssertThat(ranks.size(), Equals(positions.size()));

            uint64_t start = 0;
            for (uint64_t rank = 0; rank < positions.size(); ++rank) {
                for (; start <= positions[rank]; ++start) {
                    AssertThat(ranks.rank(start), Equals(rank));
                }
            }

            for (; start < num_bits; ++start)
                AssertThat(ranks.rank(start), Equals(positions.size()));
        });

        it("should give correct select results (very sparse)", []() {

            std::vector<uint64_t> positions{100,  200,   222,
                                            1024, 10000, 1331337};
            uint64_t num_bits = 2000000;

            filesystem::remove_all("sarray-unit-test");
            auto storage = make_sarray("sarray-unit-test", positions.begin(),
                                       positions.end(), num_bits);
            sarray_select select{"sarray-unit-test", storage};

            AssertThat(select.size(), Equals(positions.size()));

            uint64_t i = 0;
            for (const auto& pos : positions)
                AssertThat(select.select(i++), Equals(pos));
        });

        it("should give correct rank results (less sparse)", []() {
            uint64_t num_bits = 2000000;
            uint64_t stride = 100000;
            std::vector<uint64_t> positions;
            positions.reserve(num_bits / stride);
            for (uint64_t b = 0; b < num_bits; b += stride)
                positions.push_back(b);

            filesystem::remove_all("sarray-unit-test");
            auto storage = make_sarray("sarray-unit-test", positions.begin(),
                                       positions.end(), num_bits);
            sarray_rank ranks{"sarray-unit-test", storage};

            AssertThat(ranks.size(), Equals(positions.size()));

            uint64_t start = 0;
            for (uint64_t rank = 0; rank < positions.size(); ++rank) {
                for (; start <= positions[rank]; ++start) {
                    AssertThat(ranks.rank(start), Equals(rank));
                }
            }

            for (; start < num_bits; ++start)
                AssertThat(ranks.rank(start), Equals(positions.size()));
        });

        it("should give correct select results (less sparse)", []() {
            uint64_t num_bits = 2000000;
            uint64_t stride = 100000;
            std::vector<uint64_t> positions;
            positions.reserve(num_bits / stride);
            for (uint64_t b = 0; b < num_bits; b += stride)
                positions.push_back(b);

            filesystem::remove_all("sarray-unit-test");
            auto storage = make_sarray("sarray-unit-test", positions.begin(),
                                       positions.end(), num_bits);
            sarray_select select{"sarray-unit-test", storage};

            AssertThat(select.size(), Equals(positions.size()));

            uint64_t i = 0;
            for (const auto& pos : positions)
                AssertThat(select.select(i++), Equals(pos));
        });
    });
});
