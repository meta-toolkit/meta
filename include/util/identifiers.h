/**
 * @file identifiers.h
 * Defines CRTP base classes that allow for the creation of type-safe
 * "typedef" classes that serve as identifiers in the project.
 * 
 * @author Chase Geigle
 */

#ifndef _NUMERIC_IDENTIFIER_H_
#define _NUMERIC_IDENTIFIER_H_

#include <functional> // for std::hash
#include "util/comparable.h"

namespace meta {
namespace util {

/**
 * Empty helper class to denote that something is numeric.
 */
struct numeric {};

/**
 * Helper class that allows the wrapped type to be hashed into standard
 * library continers such as unordered_map or unordered_set.
 */
template <template <class> class Wrapped>
struct hash_wrapper : public Wrapped<hash_wrapper<Wrapped>> {
    using Wrapped<hash_wrapper>::Wrapped;
    using Wrapped<hash_wrapper>::operator=;
};

/**
 * CRTP base template that denotes an identifier. identifiers are
 * comparable through normal relational operators defined on the underlying
 * type T.
 */
template <class Derived, class T>
struct identifier : public comparable<identifier<Derived, T>> {
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
   explicit identifier(const T & t) : id_{ t } { }

   /**
    * identifier has a default constructor.
    */
   identifier() = default;

   /**
    * identifiers may be copy constructed.
    */
   identifier(const identifier &) = default;

   /**
    * identifiers may be copy assigned.
    */
   identifier & operator=(const identifier &) = default;

   /**
    * identifiers may be move assigned.
    */
   identifier & operator=(identifier &&) = default;

   /**
    * identifiers may be assigned into from their base type, *provided
    * they have already been constructed*.
    *
    * Example:
    * ~~~cpp
    * my_ident x{1};
    * x = 2; // fine
    * 
    * my_ident y = 1; // not fine, compiler error
    * ~~~
    */
   identifier & operator=(const T & t) { id_ = t; return *this; }

   /**
    * identifiers may be converted into their base type. This is the const
    * version.
    */
   operator const T & () const { return id_; }

   /**
    * identifiers may be converted into their base type. This is the
    * non-const version.
    */
   operator T & () { return id_; }

   /**
    * identifiers are comparable by their base types. This allows for
    * storage in comparison-based containers like std::map or std::set.
    * 
    * @param other the other identifier to compare with.
    */
   inline friend bool operator<(const identifier & lhs, 
                                const identifier & rhs) { 
      return static_cast<T>(lhs) < static_cast<T>(rhs);
   }

   /**
    * identifiers may be printed to output streams.
    */
   inline friend std::ostream & operator<<(std::ostream & stream,
                                           const identifier & ident) {
      return stream << static_cast<T>(ident);
   }

   /**
    * identifiers may be read from input streams.
    */
   inline friend std::istream & operator>>(std::istream & stream,
                                           identifier & ident) {
      return stream >> ident.id_;
   }
};

/**
 * A CRTP template base that adds numeric functionality to the identifier
 * type. numerical_identifiers support typical integer math functions on
 * top of the things supported by identifiers.
 */
template <class Derived, class T>
struct numerical_identifier: public identifier<Derived, T>, numeric {
    using identifier<Derived, T>::identifier;
    using identifier<Derived, T>::id_;
    using identifier<Derived, T>::operator=;

    Derived & operator++() { ++id_; return *derived(); }
    Derived operator++(int) { Derived t = *derived(); ++(*this); return t; }
    Derived & operator--() { --id_; return *derived(); }
    Derived operator--(int) { Derived t = *derived(); --(*this); return t; }

    Derived & operator+=(const T & step) { id_ += step; return *derived(); }
    Derived & operator-=(const T & step) { id_ -= step; return *derived(); }

    friend inline Derived operator+(Derived lhs, const Derived & rhs) {
        lhs += static_cast<T>(rhs);
        return lhs;
    }
    
    friend inline Derived operator-(Derived lhs, const Derived & rhs) {
        lhs -= static_cast<T>(rhs);
        return lhs;
    }

    private:
        Derived * derived() { return static_cast<Derived *>(this); }
};

}
}

namespace std {

/**
 * A partial specialization that allows for hashing of hash_wrapper types
 * based on their base type.
 */ 
template <template <class> class Wrapped>
struct hash<meta::util::hash_wrapper<Wrapped>> {
    template <class T>
    size_t operator()(
            const meta::util::identifier<
                      meta::util::hash_wrapper<Wrapped>, T> & to_hash) const 
    {
        return hash<T>{}(static_cast<T>(to_hash));
    }
};

}

#define MAKE_IDENTIFIER(ident_name, base_type)\
   template <class Wrapper>\
   struct ident_name##_dummy: public meta::util::identifier<Wrapper, base_type>\
   {\
      using meta::util::identifier<Wrapper, base_type>::identifier;\
      using meta::util::identifier<Wrapper, base_type>::operator=;\
   };\
   using ident_name = meta::util::hash_wrapper<ident_name##_dummy>;

#define MAKE_NUMERIC_IDENTIFIER(ident_name, base_type)\
   template <class Wrapper>\
   struct ident_name##_dummy:\
      public meta::util::numerical_identifier<Wrapper, base_type>\
   {\
      using meta::util::numerical_identifier<Wrapper,\
                                             base_type>::numerical_identifier;\
      using meta::util::numerical_identifier<Wrapper, base_type>::operator=;\
   };\
   using ident_name = meta::util::hash_wrapper<ident_name##_dummy>;

#endif
