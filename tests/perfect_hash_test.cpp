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

        using mph_builder = hashing::perfect_hash_builder<std::string>;
        using options_type = mph_builder::options;

        it("should generate minimum perfect hash functions on strings", []() {
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
    });
});
