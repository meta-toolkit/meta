/**
 * @file range.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Chase Geigle
 */

#ifndef _OPTIONAL_H_
#define _OPTIONAL_H_

#include <stdexcept>
#include <type_traits>

namespace meta {
namespace util {

constexpr struct trivial_init_t{} trivial_init{};

struct nullopt_t {
    struct init{};
    constexpr nullopt_t(init){};
};
constexpr nullopt_t nullopt{nullopt_t::init{}};

struct optional_dummy_t{};

template <class T>
union optional_storage {
    optional_dummy_t dummy_;
    T value_;

    optional_storage(trivial_init_t);

    template <class... Args>
    optional_storage(Args &&... args);

    ~optional_storage() { /* nothing */ }
};

template <class T>
class optional {
    public:
        optional();

        optional(nullopt_t);

        optional(const T & value);

        optional(T && value);

        optional(const optional & opt);

        optional(optional && opt);

        optional & operator=(optional rhs);

        ~optional();

        void swap(optional & other);

        const T & operator*() const;

        const T & value() const;

        explicit operator bool() const;

        void clear();
    private:
        T * dataptr() const;
        T * dataptr();
        bool initialized_;
        optional_storage<T> storage_;
};

class bad_optional_access : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
};

}
}

#include "util/optional.tcc"
#endif
