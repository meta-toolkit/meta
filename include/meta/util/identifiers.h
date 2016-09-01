/**
 * @file identifiers.h
 * @author Chase Geigle
 * Defines CRTP base classes that allow for the creation of type-safe
 * "typedef" classes that serve as identifiers in the project.
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_NUMERIC_IDENTIFIER_H_
#define META_NUMERIC_IDENTIFIER_H_

#include <functional> // for std::hash

#include "meta/config.h"
#include "meta/hashing/hash.h"
#include "meta/util/comparable.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace util
{

/**
 * Empty helper class to denote that something is numeric.
 */
struct numeric
{
};

/**
 * Type trait for numeric.
 */
template <class T>
struct is_numeric
{
    const static constexpr bool value
        = std::is_integral<T>::value || std::is_base_of<numeric, T>::value;
};

/**
 * Base template that denotes an identifier. identifiers are comparable
 * through normal relational operators defined on the underlying type T.
 */
template <class Tag, class T>
struct identifier : public comparable<identifier<Tag, T>>
{
    using underlying_type = T;

    /**
     * The underlying id for the identifier.
     */
    T id_;

    /**
     * identifiers must be explicitly constructed from their base
     * type---they *cannot* be implicitly converted from them!
     *
     * @param t the underlying type to convert into an identifier
     */
    explicit constexpr identifier(const T& t) : id_{t}
    {
        // nothing
    }

    /**
     * identifier has a default constructor.
     */
    identifier() = default;

    /**
     * identifiers may be copy constructed.
     */
    identifier(const identifier&) = default;

    /**
     * identifiers may be move constructed.
     */
    identifier(identifier&&) = default;

    /**
     * identifiers may be copy assigned.
     */
    identifier& operator=(const identifier&) = default;

    /**
     * identifiers may be move assigned.
     */
    identifier& operator=(identifier&&) = default;

    /**
     * identifiers may be converted into their base type. This is the const
     * version.
     *
     * @return the base type representation for this identifier
     */
    constexpr operator const T&() const
    {
        return id_;
    }

    /**
     * identifiers may be converted into their base type. This is the
     * non-const version.
     *
     * @return the base type representation for this identifier
     */
    operator T&()
    {
        return id_;
    }

    /**
     * Conversion to string_view. Enabled only if T is a std::string.
     * @return the base type representation of this identifier, converted
     *  to a string_view
     */
    template <
        typename U = T,
        typename
        = typename std::enable_if<std::is_same<U, std::string>::value>::type>
    constexpr operator util::string_view() const
    {
        return id_;
    }

    /**
     * identifiers are comparable by their base types. This allows for
     * storage in comparison-based containers like std::map or std::set.
     *
     * @param lhs
     * @param rhs
     * @return whether lhs < rhs based on T::operator<.
     */
    inline friend constexpr bool operator<(const identifier& lhs,
                                           const identifier& rhs)
    {
        return static_cast<T>(lhs) < static_cast<T>(rhs);
    }

    /**
     * identifiers may be printed to output streams.
     * @param stream The stream to write to
     * @param ident The identifier to write to the stream
     * @return `stream`
     */
    inline friend std::ostream& operator<<(std::ostream& stream,
                                           const identifier& ident)
    {
        return stream << static_cast<T>(ident);
    }

    /**
     * identifiers may be read from input streams.
     * @param stream The stream to read from
     * @param ident The identifier to read into
     * @return `stream`
     */
    inline friend std::istream& operator>>(std::istream& stream,
                                           identifier& ident)
    {
        return stream >> ident.id_;
    }
};

template <class HashAlgorithm, class Tag, class T>
void hash_append(HashAlgorithm& h, const identifier<Tag, T>& id)
{
    using util::hash_append;
    hash_append(h, static_cast<const T&>(id));
}

/**
 * A CRTP template base that adds numeric functionality to the identifier
 * type. numerical_identifiers support typical integer math functions on
 * top of the things supported by identifiers.
 */
template <class Tag, class T>
struct numerical_identifier : public identifier<Tag, T>, numeric
{
    using identifier<Tag, T>::identifier;
    using identifier<Tag, T>::id_;
    using identifier<Tag, T>::operator=;

    /**
     * Prefix-increment.
     * @return the current identifier after being incremented
     */
    numerical_identifier& operator++()
    {
        ++id_;
        return *this;
    }

    /**
     * Postifx-increment.
     * @return the old value of the identifier
     */
    numerical_identifier operator++(int)
    {
        auto t = *this;
        ++(*this);
        return t;
    }

    /**
     * Prefix-decrement.
     * @return the current identifier after being decremented
     */
    numerical_identifier& operator--()
    {
        --id_;
        return *this;
    }

    /**
     * Postfix-decrement.
     * @return the old value of the identifier
     */
    numerical_identifier operator--(int)
    {
        auto t = *this;
        --(*this);
        return t;
    }

    /**
     * @param step How much to increase the current identifier by
     * @return the current identifier
     */
    template <class U, class = typename std::
                           enable_if<std::is_convertible<U, T>::value>::type>
    numerical_identifier& operator+=(const T& step)
    {
        id_ += step;
        return *this;
    }

    /**
     * @param step How much to decrease the current identifier by
     * @return the current identifier
     */
    template <class U, class = typename std::
                           enable_if<std::is_convertible<U, T>::value>::type>
    numerical_identifier& operator-=(const T& step)
    {
        id_ -= step;
        return *this;
    }

    /**
     * @param lhs
     * @param rhs
     * @return lhs + rhs, defined in terms of
     * numerical_identifier::operator+=.
     */
    friend inline numerical_identifier
    operator+(numerical_identifier lhs, const numerical_identifier& rhs)
    {
        lhs += static_cast<T>(rhs);
        return lhs;
    }

    /**
     * @param lhs
     * @param rhs
     * @return lhs - rhs, defined in terms of
     * numerical_identifier::operator-=.
     */
    friend inline numerical_identifier
    operator-(numerical_identifier lhs, const numerical_identifier& rhs)
    {
        lhs -= static_cast<T>(rhs);
        return lhs;
    }
};
}
}

namespace std
{

/**
 * A partial specialization that allows for hashing of identifier types
 * based on their base type.
 */
template <class Tag, class T>
struct hash<meta::util::identifier<Tag, T>>
{
    /**
     * @param to_hash The identifier to be hashed
     * @return the hash code for the given identifier, defined in terms of
     * std::hash of its underlying type
     */
    size_t operator()(const meta::util::identifier<Tag, T>& to_hash) const
    {
        return hash<T>{}(static_cast<T>(to_hash));
    }
};

/**
 * A partial specialization that allows for hashing of
 * numerical_identifier types based on their base type.
 */
template <class Tag, class T>
struct hash<meta::util::numerical_identifier<Tag, T>>
{
    /**
     * @param to_hash The identifier to be hashed
     * @return the hash code for the given identifier, defined in terms of
     * std::hash of its underlying type
     */
    size_t
    operator()(const meta::util::numerical_identifier<Tag, T>& to_hash) const
    {
        return hash<T>{}(static_cast<T>(to_hash));
    }
};
}

#define MAKE_USER_DEFINED_LITERAL(ident_name, base_type, suffix)               \
    inline ident_name operator"" suffix(const char* str, std::size_t len)      \
    {                                                                          \
        return ident_name{base_type{str, len}};                                \
    }

#define MAKE_USER_DEFINED_NUMERIC_LITERAL(ident_name, base_type, suffix)       \
    inline ident_name operator"" suffix(unsigned long long int val)            \
    {                                                                          \
        return ident_name{base_type(val)};                                     \
    }

#define MAKE_OPAQUE_IDENTIFIER(ident_name, base_type)                          \
    struct ident_name##_tag                                                    \
    {                                                                          \
    };                                                                         \
    using ident_name = meta::util::identifier<ident_name##_tag, base_type>;

#define MAKE_OPAQUE_NUMERIC_IDENTIFIER(ident_name, base_type)                  \
    struct ident_name##_tag                                                    \
    {                                                                          \
    };                                                                         \
    using ident_name                                                           \
        = meta::util::numerical_identifier<ident_name##_tag, base_type>;

#if !defined NDEBUG && !defined NUSE_OPAQUE_IDENTIFIERS
#define MAKE_IDENTIFIER(ident_name, base_type)                                 \
    MAKE_OPAQUE_IDENTIFIER(ident_name, base_type)
#else
#define MAKE_IDENTIFIER(ident_name, base_type) using ident_name = base_type;
#endif

#if !defined NDEBUG && !defined NUSE_OPAQUE_IDENTIFIERS
#define MAKE_NUMERIC_IDENTIFIER(ident_name, base_type)                         \
    MAKE_OPAQUE_NUMERIC_IDENTIFIER(ident_name, base_type)
#else
#define MAKE_NUMERIC_IDENTIFIER(ident_name, base_type)                         \
    using ident_name = base_type;
#endif

#if !defined NDEBUG && !defined NUSE_OPAQUE_IDENTIFIERS
#define MAKE_IDENTIFIER_UDL(ident_name, base_type, suffix)                     \
    MAKE_OPAQUE_IDENTIFIER(ident_name, base_type)                              \
    MAKE_USER_DEFINED_LITERAL(ident_name, base_type, suffix)
#else
#define MAKE_IDENTIFIER_UDL(ident_name, base_type, suffix)                     \
    using ident_name = base_type;                                              \
    MAKE_USER_DEFINED_LITERAL(ident_name, base_type, suffix)
#endif

#if !defined NDEBUG && !defined NUSE_OPAQUE_IDENTIFIERS
#define MAKE_NUMERIC_IDENTIFIER_UDL(ident_name, base_type, suffix)             \
    MAKE_OPAQUE_NUMERIC_IDENTIFIER(ident_name, base_type)                      \
    MAKE_USER_DEFINED_NUMERIC_LITERAL(ident_name, base_type, suffix)
#else
#define MAKE_NUMERIC_IDENTIFIER_UDL(ident_name, base_type, suffix)             \
    using ident_name = base_type;                                              \
    MAKE_USER_DEFINED_NUMERIC_LITERAL(ident_name, base_type, suffix)
#endif

#endif
