/**
 * @file darray_test.cpp
 * @author Chase Geigle
 */

#include "bandit/bandit.h"
#include "meta/io/filesystem.h"
#include "meta/math/integer.h"
#include "meta/succinct/darray.h"

using namespace bandit;

go_bandit([]() {
    using namespace meta;
    using namespace succinct;

    describe("[darray]", []() {
        filesystem::remove_all("darray-unit-test");

        it("should correctly locate one bits in small blocks", []() {
            auto alternating_ones
                = static_cast<uint64_t>(0xaaaaaaaaaaaaaaaaULL);

            std::vector<std::size_t> sizes(128000, 64);
            uint64_t total_size
                = std::accumulate(sizes.begin(), sizes.end(), 0ull);

            std::vector<uint64_t> storage;
            storage.reserve(math::integer::div_ceil(total_size, 64));

            {
                auto builder = make_bit_vector_builder(
                    [&](uint64_t word) { storage.push_back(word); });

                for (const auto& size : sizes)
                    builder.write_bits(
                        {alternating_ones, static_cast<uint8_t>(size)});
            }
            AssertThat(storage.size(),
                       Equals(math::integer::div_ceil(total_size, 64)));

            bit_vector_view bvv{{storage}, total_size};

            darray1 ones{"darray-unit-test", bvv};

            AssertThat(ones.num_positions(), Equals(total_size / 2));

            // there is a one in every other position
            for (std::size_t i = 0; i < total_size / 2; ++i) {
                AssertThat(ones.select(i), Equals(i * 2 + 1));
            }
        });

        it("should correctly locate one bits in oddly-sized vector", []() {
            uint64_t deadbeef = 0xdeadbeefULL;
            auto sizes = {32, 16, 64, 38, 32, 64, 8, 1, 2, 3, 7, 9};
            uint64_t total_size
                = std::accumulate(sizes.begin(), sizes.end(), 0ull);

            std::vector<uint64_t> storage;
            {
                auto builder = make_bit_vector_builder(
                    [&](uint64_t word) { storage.push_back(word); });

                for (const auto& size : sizes)
                    builder.write_bits({deadbeef, static_cast<uint8_t>(size)});
            }

            bit_vector_view bvv{{storage}, total_size};

            {
                filesystem::remove_all("darray-unit-test");
                darray1 ones{"darray-unit-test", bvv};

                uint64_t rank_pos = 0;
                for (uint64_t i = 0; i < total_size; ++i) {
                    if (bvv[i]) {
                        AssertThat(ones.select(rank_pos), Equals(i));
                        ++rank_pos;
                    }
                }
            }
        });

        it("should correctly locate zero bits in oddly-sized vector", []() {
            uint64_t deadbeef = 0xdeadbeefULL;
            auto sizes = {32, 16, 64, 38, 32, 64, 8, 1, 2, 3, 7, 9};
            uint64_t total_size
                = std::accumulate(sizes.begin(), sizes.end(), 0ull);

            std::vector<uint64_t> storage;
            {
                auto builder = make_bit_vector_builder(
                    [&](uint64_t word) { storage.push_back(word); });

                for (const auto& size : sizes)
                    builder.write_bits({deadbeef, static_cast<uint8_t>(size)});
            }

            bit_vector_view bvv{{storage}, total_size};

            {
                filesystem::remove_all("darray-unit-test");
                darray0 zeroes{"darray-unit-test", bvv};

                uint64_t rank_pos = 0;
                for (uint64_t i = 0; i < total_size; ++i) {
                    if (!bvv[i]) {
                        AssertThat(zeroes.select(rank_pos), Equals(i));
                        ++rank_pos;
                    }
                }
            }
        });

        it("should correctly locate one bits in large blocks", []() {
            auto sparse_pattern = 1ULL;
            auto zero_pattern = 0ULL;

            std::vector<std::size_t> sizes(128000, 64);
            uint64_t total_size
                = std::accumulate(sizes.begin(), sizes.end(), 0ull);

            std::vector<uint64_t> storage;
            storage.reserve(math::integer::div_ceil(total_size, 64));

            {
                auto builder = make_bit_vector_builder(
                    [&](uint64_t word) { storage.push_back(word); });

                for (std::size_t i = 0; i < sizes.size(); ++i) {
                    if (i % 2 == 0) {
                        builder.write_bits(
                            {sparse_pattern, static_cast<uint8_t>(sizes[i])});
                    } else {
                        builder.write_bits(
                            {zero_pattern, static_cast<uint8_t>(sizes[i])});
                    }
                }
            }
            AssertThat(storage.size(),
                       Equals(math::integer::div_ceil(total_size, 64)));

            bit_vector_view bvv{{storage}, total_size};

            filesystem::remove_all("darray-unit-test");
            darray1 ones{"darray-unit-test", bvv};
            AssertThat(ones.num_positions(), Equals(uint64_t{64000}));
            for (uint64_t i = 0; i < 64000; ++i) {
                AssertThat(ones.select(i), Equals(128 * i));
            }
        });

        it("should locate one bits in a random bit vector", []() {
            std::vector<uint64_t> storage(128000);

            std::mt19937_64 rng{47};
            std::generate(storage.begin(), storage.end(),
                          [&]() { return rng(); });

            bit_vector_view bvv{{storage}, 128000 * 64};

            filesystem::remove_all("darray-unit-test");
            darray1 ones{"darray-unit-test", bvv};

            uint64_t rank_idx = 0;
            for (uint64_t i = 0; i < 128000 * 64; ++i) {
                if (bvv[i]) {
                    auto pos = ones.select(rank_idx);
                    AssertThat(pos, Equals(i));
                    ++rank_idx;
                }
            }
        });

        it("should locate zero bits in a random bit vector", []() {
            std::vector<uint64_t> storage(128000);

            std::mt19937_64 rng{47};
            std::generate(storage.begin(), storage.end(),
                          [&]() { return rng(); });

            bit_vector_view bvv{{storage}, 128000 * 64};

            filesystem::remove_all("darray-unit-test");
            darray0 zeroes{"darray-unit-test", bvv};

            uint64_t rank_idx = 0;
            for (uint64_t i = 0; i < 128000 * 64; ++i) {
                if (!bvv[i]) {
                    auto pos = zeroes.select(rank_idx);
                    AssertThat(pos, Equals(i));
                    ++rank_idx;
                }
            }
        });
    });
});
