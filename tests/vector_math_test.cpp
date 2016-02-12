/**
 * @file vector_math_test.h
 * @author Chase Geigle
 */

#include "bandit/bandit.h"
#include "meta/math/vector.h"

using namespace bandit;

go_bandit([]() {
    describe("[vector math]", []() {
        using namespace meta;
        using namespace math::operators;
        std::vector<int> a = {2, 2, 2, 2};
        std::vector<int> b = {1, 1, 1, 1};

        it("should add two vectors (const ref + const ref)", [&]() {
            auto c = a + b;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (rval + const ref)", [&]() {
            auto c = std::vector<int>(a) + b;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (const ref + rval)", [&]() {
            auto c = a + std::vector<int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (rval + rval)", [&]() {
            auto c = std::vector<int>(a) + std::vector<int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (const ref + array_view)", [&]() {
            auto c = a + util::array_view<const int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (rval + array_view)", [&]() {
            auto c = std::vector<int>(a) + util::array_view<const int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (array_view + const ref)", [&]() {
            auto c = util::array_view<const int>(a) + b;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (array_view + rval)", [&]() {
            auto c = util::array_view<const int>(a) + std::vector<int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (array_view<T> + array_view<T>)", [&]() {
            auto c = util::array_view<const int>(a)
                     + util::array_view<const int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(3));
        });

        it("should add two vectors (array_view<const T> + array_view<T>)",
           [&]() {
               auto c
                   = util::array_view<const int>(a) + util::array_view<int>(b);
               AssertThat(c.size(), Equals(a.size()));
               for (const auto& v : c)
                   AssertThat(v, Equals(3));
           });

        it("should subtract two vectors (const ref - const ref)", [&]() {
            auto c = a - b;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should subtract two vectors (rval - const ref)", [&]() {
            auto c = std::vector<int>(a) - b;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should subtract two vectors (const ref - rval)", [&]() {
            auto c = a - std::vector<int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should subtract two vectors (rval - rval)", [&]() {
            auto c = std::vector<int>(a) - std::vector<int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should subtract two vectors (const ref - array_view)", [&]() {
            auto c = a - util::array_view<const int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should subtract two vectors (rval - array_view)", [&]() {
            auto c = std::vector<int>(a) - util::array_view<const int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should subtract two vectors (array_view - const ref)", [&]() {
            auto c = util::array_view<const int>(a) - b;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should subtract two vectors (array_view - rval)", [&]() {
            auto c = util::array_view<const int>(a) - std::vector<int>(b);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should subtract two vectors (array_view<T> - array_view<T>)",
           [&]() {
               auto c = util::array_view<const int>(a)
                        - util::array_view<const int>(b);
               AssertThat(c.size(), Equals(a.size()));
               for (const auto& v : c)
                   AssertThat(v, Equals(1));
           });

        it("should add two vectors (array_view<const T> - array_view<T>)",
           [&]() {
               auto c
                   = util::array_view<const int>(a) - util::array_view<int>(b);
               AssertThat(c.size(), Equals(a.size()));
               for (const auto& v : c)
                   AssertThat(v, Equals(1));
           });

        it("should divide by a constant (const ref / scalar)", [&]() {
            auto c = a / 2;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should divide by a constant (rval / scalar)", [&]() {
            auto c = std::vector<int>(a) / 2;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should divide by a constant (array_view / scalar)", [&]() {
            auto c = util::array_view<const int>(a) / 2;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(1));
        });

        it("should multiply by a constant (const ref * scalar)", [&]() {
            auto c = a * 2;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(4));
        });

        it("should multiply by a constant (rval * scalar)", [&]() {
            auto c = std::vector<int>(a) * 2;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(4));
        });

        it("should multiply by a constant (array_view * scalar)", [&]() {
            auto c = util::array_view<const int>(a) * 2;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(4));
        });

        it("should multiply by a constant (scalar * const ref)", [&]() {
            auto c = 2 * a;
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(4));
        });

        it("should multiply by a constant (scalar * rval)", [&]() {
            auto c = 2 * std::vector<int>(a);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(4));
        });

        it("should multiply by a constant (scalar * array_view)", [&]() {
            auto c = 2 * std::vector<int>(a);
            AssertThat(c.size(), Equals(a.size()));
            for (const auto& v : c)
                AssertThat(v, Equals(4));
        });

        it("should compute the l2-norm (const ref)", [&]() {
            auto val = l2norm(a);
            AssertThat(val, Equals(std::sqrt(2 * 2 + 2 * 2 + 2 * 2 + 2 * 2)));
        });

        it("should compute the l2-norm (array_view)", [&]() {
            auto val = l2norm(util::array_view<const int>(a));
            AssertThat(val, Equals(std::sqrt(2 * 2 + 2 * 2 + 2 * 2 + 2 * 2)));
        });

        it("should compute the l1-norm (const ref)", [&]() {
            auto val = l1norm(a);
            AssertThat(val, Equals(2 + 2 + 2 + 2));
        });

        it("should compute the l1-norm (const ref)", [&]() {
            auto val = l1norm(util::array_view<const int>(a));
            AssertThat(val, Equals(2 + 2 + 2 + 2));
        });
    });
});
