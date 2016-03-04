/**
 * @file bit_vector_test.cpp
 * @author Chase Geigle
 */

#include <fstream>

#include "bandit/bandit.h"
#include "meta/io/filesystem.h"
#include "meta/math/integer.h"
#include "meta/succinct/bit_vector.h"
#include "meta/util/disk_vector.h"

using namespace bandit;

go_bandit([]() {
    using namespace meta;
    using namespace succinct;

    describe("[bit vector]", []() {
        std::string filename = "bit-vector-test.bin";
        it("should build files of the correct size", [&]() {
            {
                std::ofstream output{filename, std::ios::binary};

                auto builder = make_bit_vector_builder(output);

                auto all_ones = static_cast<uint64_t>(-1);
                builder.write_bits({all_ones, 16});
                builder.write_bits({all_ones, 64});

                AssertThat(builder.total_bits(),
                           Equals(static_cast<uint64_t>(16 + 64)));
            }

            AssertThat(filesystem::file_size(filename),
                       Equals(sizeof(uint64_t) * 2));
            filesystem::delete_file(filename);
        });

        it("should correctly extract single bits", [&]() {
            auto alternating_ones
                = static_cast<uint64_t>(0xaaaaaaaaaaaaaaaaULL);

            auto sizes = {16, 8, 64, 2, 16, 32, 4};
            uint64_t total_size
                = std::accumulate(sizes.begin(), sizes.end(), 0ull);
            {
                std::ofstream output{filename, std::ios::binary};

                auto builder = make_bit_vector_builder(output);

                for (const auto& size : sizes)
                    builder.write_bits(
                        {alternating_ones, static_cast<uint8_t>(size)});

                AssertThat(builder.total_bits(), Equals(total_size));
            }

            AssertThat(filesystem::file_size(filename),
                       Equals(sizeof(uint64_t)
                              * math::integer::div_ceil(total_size, 64)));

            {
                util::disk_vector<uint64_t> storage{filename};
                util::array_view<const uint64_t> av{storage.begin(),
                                                    storage.end()};
                bit_vector_view bvv{av, total_size};

                for (std::size_t i = 0; i < total_size; ++i) {
                    auto bit = bvv[i];
                    if (i % 2 == 0) {
                        AssertThat(bit, Equals(0));
                    } else {
                        AssertThat(bit, Equals(1));
                    }
                }
            }

            filesystem::delete_file(filename);
        });

        it("should correctly extract multi-bit patterns", [&]() {
            uint64_t deadbeef = 0xdeadbeefULL;
            auto sizes = {32, 16, 64, 38, 32, 64, 8, 1, 2, 3, 7, 9};
            uint64_t total_size
                = std::accumulate(sizes.begin(), sizes.end(), 0ull);
            {
                std::ofstream output{filename, std::ios::binary};

                auto builder = make_bit_vector_builder(output);
                for (const auto& size : sizes)
                    builder.write_bits({deadbeef, static_cast<uint8_t>(size)});

                AssertThat(builder.total_bits(), Equals(total_size));
            }

            AssertThat(filesystem::file_size(filename),
                       Equals(sizeof(uint64_t)
                              * math::integer::div_ceil(total_size, 64)));
            {
                util::disk_vector<uint64_t> storage{filename};
                util::array_view<const uint64_t> av{storage.begin(),
                                                    storage.end()};

                bit_vector_view bvv{av, total_size};

                uint64_t pos = 0;
                for (const auto& sze : sizes) {
                    auto size = static_cast<uint8_t>(sze);
                    auto result = bvv.extract(pos, size);

                    auto mask = size == 64 ? static_cast<uint64_t>(-1)
                                           : (1ull << size) - 1;
                    AssertThat(result, Equals(deadbeef & mask));

                    pos += size;
                }
            }
            filesystem::delete_file(filename);
        });
    });
});
