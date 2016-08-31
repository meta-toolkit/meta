/**
 * @file perfect_hash_test.cpp
 * @author Chase Geigle
 */

#include <string>

#include "bandit/bandit.h"
#include "meta/hashing/perfect_hash.h"
#include "meta/hashing/perfect_hash_builder.h"
#include "meta/io/filesystem.h"

using namespace bandit;

go_bandit([]() {
    using namespace meta;
    using namespace hashing;

    describe("[perfect hash]", []() {
        it("should generate minimum perfect hash functions on strings", []() {
            using mph_builder = hashing::perfect_hash_builder<std::string>;
            using options_type = mph_builder::options;
            filesystem::remove_all("perfect-hash-unit-test");

            options_type options;
            options.prefix = "perfect-hash-unit-test";
            options.num_keys
                = filesystem::num_lines("../data/lemur-stopwords.txt");
            options.max_ram = 1024 * 1024; // 1MB

            {

                mph_builder builder{options};

                std::ifstream input{"../data/lemur-stopwords.txt"};
                std::string line;
                while (std::getline(input, line))
                    builder(line);

                builder.write();
            }

            {
                hashing::perfect_hash<std::string> mph{
                    "perfect-hash-unit-test"};

                std::vector<std::string> vocab(options.num_keys);

                std::ifstream input{"../data/lemur-stopwords.txt"};
                std::string line;
                while (std::getline(input, line)) {
                    auto id = mph(line);
                    AssertThat(vocab[id].empty(), Is().True());
                    vocab[id] = line;
                }

                for (std::size_t id = 0; id < vocab.size(); ++id) {
                    AssertThat(vocab[id].empty(), Is().False());
                }
            }

            filesystem::remove_all("perfect-hash-unit-test");
        });

        it("should generate perfect hash functions on vectors of ints", []() {
            using mph_builder
                = hashing::perfect_hash_builder<std::vector<uint64_t>>;
            using options_type = mph_builder::options;
            filesystem::remove_all("perfect-hash-unit-test");

            std::vector<std::vector<uint64_t>> keys
                = {{1, 2, 3},
                   {4, 5, 6},
                   {1489237, 1930481390, 1394483},
                   {7, 839, 2019},
                   {1129, 219, 1}};

            options_type options;
            options.prefix = "perfect-hash-unit-test";
            options.num_keys = keys.size();
            options.max_ram = 1024 * 1024; // 1MB
            {
                mph_builder builder{options};

                for (const auto& vec : keys)
                    builder(vec);

                builder.write();
            }

            {
                hashing::perfect_hash<std::vector<uint64_t>> mph{
                    options.prefix};

                std::vector<uint64_t> indices;
                for (const auto& key : keys) {
                    auto h = mph(key);
                    AssertThat(h, Is().GreaterThanOrEqualTo(uint64_t{0}));
                    AssertThat(h, Is().LessThan(keys.size()));
                    indices.push_back(mph(key));
                }

                std::sort(indices.begin(), indices.end());
                AssertThat(std::adjacent_find(indices.begin(), indices.end()),
                           Is().EqualTo(indices.end()));
            }

            filesystem::remove_all("perfect-hash-unit-test");
        });

    });
});
