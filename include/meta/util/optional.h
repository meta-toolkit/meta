/**
 * @file optional.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_OPTIONAL_H_
#define META_UTIL_OPTIONAL_H_

#include "meta/config.h"

#if META_HAS_EXPERIMENTAL_OPTIONAL
#include <experimental/optional>
namespace meta
{
namespace util
{
template <class T>
using optional = std::experimental::optional<T>;

using std::experimental::nullopt;
}
}
#else
#include "meta/util/comparable.h"
#include <stdexcept>
#include <type_traits>

namespace meta
{
namespace util
{

/**
 * A tag for trivial initialization of optional storage.
 */
constexpr struct trivial_init_t
{
} trivial_init{};

/**
 * A dummy type for representing a disengaged option<T>.
 */
struct nullopt_t
{
    /** An empty object */
    struct init
    {
    };
    constexpr nullopt_t(init){};
};
/// A global nullopt_t constant.
constexpr nullopt_t nullopt{nullopt_t::init{}};

/**
 * A dummy type for optional storage.
 */
struct optional_dummy_t
{
};

/**
 * A storage class for the optional<T> class.
 */
template <class T>
union optional_storage {
    /**
     * A dummy value.
     */
    optional_dummy_t dummy_;

    /**
     * The contained value of an option<T> that is engaged.
     */
    T value_;

    /**
     * Trivial constructor for the storage class.
     */
    optional_storage(trivial_init_t);

    /**
     * Forwarding constructor for the storage class, to create the
     * internal value.
     *
     * @param args The arguments to forward to the internal value's
     * constructor
     */
    template <class... Args>
    optional_storage(Args&&... args);

    /**
     * no-op destructor.
     */
    ~optional_storage()
    {
        /* nothing */
    }
};

/**
 * A class for representing optional values. This is a very naiive approach
 * based on the design given in the WG21 paper and the reference
 * implementation. Many features of the real optional<T> design there are
 * not supported here---this is a lightweight replacement until C++14 is
 * completed and implemented in compilers.
 *
 * @see http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2013/n3527.html
 * @see https://github.com/akrzemi1/Optional/
 */
template <class T>
class optional : public util::comparable<optional<T>>
{
  public:
    /**
     * Default constructor, creates a disengaged optional value.
     */
    optional();

    /**
     * Conversion constructor from nullopt, creates a disengaged
     * optional value.
     */
    optional(nullopt_t);

    /**
     * Creates an optional value with the given contents.
     * @param value the desired value to be contained in the optional
     */
    optional(const T& value);

    /**
     * Creates an optional value with the given contents via move
     * construction.
     * @param value the desired value to be moved into the optional
     */
    optional(T&& value);

    /**
     * Copy constructor.
     */
    optional(const optional&);

    /**
     * Move constructor.
     */
    optional(optional&&);

    /**
     * Assignment operator.
     */
    optional& operator=(optional);

    /**
     * Destructor. Responsible for ensuring the proper destruction of
     * the contained type, if it is engaged.
     */
    ~optional();

    /**
     * Swaps the current optional instance with the parameter.
     * @param other the optional to swap with
     */
    void swap(optional& other);

    /**
     * Obtains the value contained in the optional. Const version.
     *
     * @return the value in the optional
     */
    const T& operator*() const;

    /**
     * Obtains the value contained in the optional. Non-const version.
     *
     * @return the value in the optional
     */
    T& operator*();

    /**
     * Member access operator to the value contained in the optional.
     * Const version.
     *
     * @return a pointer to the current value in the optional (member
     * access)
     */
    const T* operator->() const;

    /**
     * Member access operator to the value contained in the optional.
     * Non-const version.
     *
     * @return a pointer to the current value in the optional (member
     * access)
     */
    T* operator->();

    /**
     * @return whether the optional is engaged
     */
    explicit operator bool() const;

    /**
     * Empties the optional.
     */
    void clear();

    /**
     * @param default_value The value to return if this optional is empty
     * @return the contained value if there is on, or default_value
     * otherwise
     */
    template <class U>
    T value_or(U&& default_value) const&;

    /**
     * @param default_value The value to return if this optional is empty
     * @return the contained value if there is on, or default_value
     * otherwise
     */
    template <class U>
    T value_or(U&& default_value) &&;

  private:
    /**
     * Helper function to obtain the address of the contained value.
     * const version.
     *
     * @return the address of the contained value
     */
    const T* dataptr() const;

    /**
     * Helper function to obtain the address of the contained value.
     * non-const version.
     *
     * @return the address of the contained value
     */
    T* dataptr();

    /**
     * Whether or not this optional is engaged.
     */
    bool initialized_;

    /**
     * The storage for this optional.
     */
    optional_storage<T> storage_;
};

template <class T>
bool operator<(const optional<T>& lhs, const optional<T>& rhs);

/**
 * Exception thrown when trying to obtain the value of a non-engaged
 * optional.
 */
class bad_optional_access : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#include "meta/util/optional.tcc"
#endif // !META_HAS_EXPERIMENTAL_OPTIONAL
#endif // META_UTIL_OPTIONAL_H_
