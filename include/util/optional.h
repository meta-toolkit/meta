/**
 * @file optional.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_OPTIONAL_H_
#define META_OPTIONAL_H_

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
    constexpr nullopt_t(init) {};
};
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
union optional_storage
{
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
     */
    template <class... Args>
    optional_storage(Args&&... args);

    /**
     * no-op destructor.
     */
    ~optional_storage()
    {/* nothing */
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
class optional
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
    optional(const optional& opt);

    /**
     * Move constructor.
     */
    optional(optional&& opt);

    /**
     * Assignment operator.
     */
    optional& operator=(optional rhs);

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
     */
    const T& operator*() const;

    /**
     * Obtains the value contained in the optional. Non-const version.
     */
    T& operator*();

    /**
     * Member access operator to the value contained in the optional.
     * Const version.
     */
    const T* operator->() const;

    /**
     * Member access operator to the value contained in the optional.
     * Non-const version.
     */
    T* operator->();

    /**
     * Determines if the optional is engaged.
     */
    explicit operator bool() const;

    /**
     * Empties the optional.
     */
    void clear();

  private:
    /**
     * Helper function to obtain the address of the contained value.
     * const version.
     */
    const T* dataptr() const;

    /**
     * Helper function to obtain the address of the contained value.
     * non-const version.
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

#include "util/optional.tcc"
#endif
