/**
 * @file compressed_stream_test.cpp
 * @author Chase Geigle
 */

#include <fstream>

#include "bandit/bandit.h"
#include "meta/io/filesystem.h"
#include "meta/io/gzstream.h"

#if META_HAS_LIBLZMA
#include "meta/io/xzstream.h"
#endif

using namespace bandit;
using namespace snowhouse;
using namespace meta;

template <class CompressedOutputStream = io::gzofstream>
void compress_file(const std::string& input, const std::string& output) {
    std::ifstream input_stream{input};
    CompressedOutputStream output_stream{output};

    std::string line;
    while (std::getline(input_stream, line))
        output_stream << line << '\n';
}

void check_stream_equality(std::istream& gold, std::istream& proposed) {
    std::string gold_line;
    std::string prop_line;

    for (; gold && proposed;
         std::getline(gold, gold_line), std::getline(proposed, prop_line)) {
        AssertThat(prop_line, Equals(gold_line));
    }

    AssertThat(!gold, IsTrue());
    AssertThat(!proposed, IsTrue());
}

template <class CompressedIStream = io::gzifstream,
          class CompressedOStream = io::gzofstream>
void define_tests(const std::string& orig_file, const std::string& out_file) {
    compress_file<CompressedOStream>(orig_file, out_file);
    it("should successfully compress a small text file", [&]() {
        AssertThat(filesystem::file_exists(out_file), IsTrue());

        auto comp_size = filesystem::file_size(out_file);
        auto orig_size = filesystem::file_size(orig_file);
        AssertThat(comp_size, IsLessThan(orig_size));
    });

    it("should successfully read a compressed text file", [&]() {
        AssertThat(filesystem::file_exists(out_file), IsTrue());

        std::ifstream gold{orig_file};
        CompressedIStream input{out_file};
        check_stream_equality(gold, input);
    });

    it("should convert to false on nonexistent input file", [&]() {
        CompressedIStream input{out_file + ".fkladflah"};
        AssertThat(static_cast<bool>(input), IsFalse());
        AssertThat(!input, IsTrue());
    });

    filesystem::delete_file(out_file);
}

go_bandit([]() {
    const std::string orig_file = "../data/sample-document.txt";
    describe("[io] gzstream", [&]() {
        const std::string out_file = "sample-document.txt.gz";
        define_tests(orig_file, out_file);
    });

#if META_HAS_LIBLZMA
    describe("[io] xzstream", [&]() {
        const std::string out_file = "sample-document.txt.xz";
        define_tests<io::xzifstream, io::xzofstream>(orig_file, out_file);
    });
#endif
});
