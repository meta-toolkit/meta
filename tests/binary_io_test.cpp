/**
 * @file binary_io_test.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <algorithm>
#include <fstream>
#include <numeric>
#include <random>

#include "bandit/bandit.h"
#include "meta/classify/classifier/svm_wrapper.h" // to use kernel enum
#include "meta/io/binary.h"
#include "meta/io/filesystem.h"
#include "meta/io/packed.h"

using namespace bandit;
using namespace meta;

namespace {

template <class T>
void test_write(const T& elem, std::ofstream& outfile, bool use_packed) {
    if (use_packed)
        io::packed::write(outfile, elem);
    else
        io::write_binary(outfile, elem);
}

template <class T>
uint64_t test_read(T& read_elem, std::ifstream& infile, bool use_packed) {
    if (use_packed)
        return io::packed::read(infile, read_elem);
    io::read_binary(infile, read_elem);
    return sizeof(T); // not correct for strings, but ignored
}

template <class T>
void test_read_write(const std::vector<T>& elems, bool use_packed) {
    const std::string filename{"meta-tmp-compressed.dat"};
    {
        std::ofstream outfile{filename, std::ios::binary};
        for (const auto& elem : elems)
            test_write(elem, outfile, use_packed);
    }
    uint64_t bytes_read = 0;
    {
        std::ifstream infile{filename, std::ios::binary};
        for (const auto& elem : elems) {
            T read_elem;
            bytes_read += test_read(read_elem, infile, use_packed);
            AssertThat(read_elem, Equals(elem));
        }
    }
    // ignore size checking when writing non-packed strings
    if (use_packed || !std::is_same<std::string, T>::value)
        AssertThat(bytes_read, Equals(filesystem::file_size(filename)));
    AssertThat(filesystem::delete_file(filename), IsTrue());
}

void test_multi_read_write(bool use_packed) {
    auto elem_0 = std::string{"yap!"};
    auto elem_1 = uint64_t{47};
    auto elem_2 = int16_t{-10};
    auto elem_3 = classify::svm_wrapper::RBF;
    auto elem_4 = true;
    auto elem_5 = 0.987;
    auto elem_6 = 1.618f;
    auto elem_7 = std::string{"the end!"};
    const std::string filename{"meta-tmp-compressed.dat"};
    {
        std::ofstream outfile{filename, std::ios::binary};
        test_write(elem_0, outfile, use_packed);
        test_write(elem_1, outfile, use_packed);
        test_write(elem_2, outfile, use_packed);
        test_write(elem_3, outfile, use_packed);
        test_write(elem_4, outfile, use_packed);
        test_write(elem_5, outfile, use_packed);
        test_write(elem_6, outfile, use_packed);
        test_write(elem_7, outfile, use_packed);
    }
    std::string read_elem_0;
    uint64_t read_elem_1;
    int16_t read_elem_2;
    classify::svm_wrapper::kernel read_elem_3;
    bool read_elem_4;
    double read_elem_5;
    float read_elem_6;
    std::string read_elem_7;
    {
        std::ifstream infile{filename, std::ios::binary};
        test_read(read_elem_0, infile, use_packed);
        AssertThat(read_elem_0, Equals(elem_0));
        test_read(read_elem_1, infile, use_packed);
        AssertThat(read_elem_1, Equals(elem_1));
        test_read(read_elem_2, infile, use_packed);
        AssertThat(read_elem_2, Equals(elem_2));
        test_read(read_elem_3, infile, use_packed);
        AssertThat(read_elem_3, Equals(elem_3));
        test_read(read_elem_4, infile, use_packed);
        AssertThat(read_elem_4, Equals(elem_4));
        test_read(read_elem_5, infile, use_packed);
        AssertThat(read_elem_5, Equals(elem_5));
        test_read(read_elem_6, infile, use_packed);
        AssertThat(read_elem_6, Equals(elem_6));
        test_read(read_elem_7, infile, use_packed);
        AssertThat(read_elem_7, Equals(elem_7));
    }
    AssertThat(filesystem::delete_file(filename), IsTrue());
}
}

go_bandit([]() {

    const std::vector<double> double_elems
        = {1.0,    0.999,      -0.901341,        4e9,         1.0 / 3.0,
           12e-23, 0.00000001, -2309095951.4927, -5426987e-12};

    const std::vector<float> float_elems
        = {1.0f,    0.999f,      -0.901341f,        4e9f,         1.0f / 3.0f,
           12e-23f, 0.00000001f, -2309095951.4927f, -5426987e-12f};

    std::vector<uint32_t> uint_elems(100, 0);
    std::iota(uint_elems.begin(), uint_elems.end(), 1);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(uint_elems.begin(), uint_elems.end(), g);

    std::vector<int64_t> int_elems;
    for (const auto& elem : uint_elems)
        int_elems.push_back(static_cast<int64_t>(elem));

    const std::vector<std::string> string_elems
        = {"yap",      "yea",
           "yup",      "yep",
           "yeah",     "43967#%*&^!",
           "0",        "",
           "!!!!111!", " ",
           "\t",       "* * * nickels",
           "\r\n\t",   "=-20&^%#&E#&%#$&^%#$&%#$$#&35=25-252924-02895420458"};

    using meta::classify::svm_wrapper;
    const std::vector<svm_wrapper::kernel> enum_elems
        = {svm_wrapper::None,  svm_wrapper::Quadratic, svm_wrapper::None,
           svm_wrapper::Cubic, svm_wrapper::Cubic,     svm_wrapper::Quartic,
           svm_wrapper::RBF,   svm_wrapper::Sigmoid,   svm_wrapper::Cubic};

    describe("[binary-io] packed read and write", [&]() {

        it("should read and write doubles",
           [&]() { test_read_write(double_elems, true); });

        it("should read and write floats",
           [&]() { test_read_write(float_elems, true); });

        it("should read and write ints",
           [&]() { test_read_write(int_elems, true); });

        it("should read and write unsigned ints",
           [&]() { test_read_write(uint_elems, true); });

        it("should read and write strings",
           [&]() { test_read_write(string_elems, true); });

        it("should read and write enums",
           [&]() { test_read_write(enum_elems, true); });

        it("should read and write multiple types from the same stream",
           [&]() { test_multi_read_write(true); });
    });

    describe("[binary-io] read and write", [&]() {

        it("should read and write doubles",
           [&]() { test_read_write(double_elems, false); });

        it("should read and write floats",
           [&]() { test_read_write(float_elems, false); });

        it("should read and write ints",
           [&]() { test_read_write(int_elems, false); });

        it("should read and write unsigned ints",
           [&]() { test_read_write(uint_elems, false); });

        it("should read and write strings",
           [&]() { test_read_write(string_elems, false); });

        it("should read and write enums",
           [&]() { test_read_write(enum_elems, false); });

        it("should read and write multiple types from the same stream",
           [&]() { test_multi_read_write(false); });
    });
});
