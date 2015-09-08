/**
 * @file string_view.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_STRING_VIEW_H_
#define META_UTIL_STRING_VIEW_H_

#include "util/hash.h"

#if META_HAS_EXPERIMENTAL_STRING_VIEW
#include <experimental/string_view>
namespace meta
{
namespace util
{
template <class Char, class Traits = std::char_traits<Char>>
using basic_string_view = std::experimental::basic_string_view<Char, Traits>;

using string_view = basic_string_view<char>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;
using wstring_view = basic_string_view<wchar_t>;
}
}
#else

#include <algorithm>
#include <stdexcept>
#include <string>

namespace meta
{
namespace util
{

/**
 * A non-owning reference to a string. I make no claims that this is
 * completely standards-compliant---this is just a best-effort attempt at
 * implementing what we need for MeTA. I have built this using its paper's
 * wording for the Fundamentals TS.
 *
 * @see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3921.html
 */
template <class Char, class Traits = std::char_traits<Char>>
class basic_string_view
{
  public:
    using traits_type = Traits;
    using value_type = Char;
    using pointer = Char*;
    using const_pointer = const Char*;
    using reference = Char&;
    using const_reference = const Char&;
    using const_iterator = const_pointer;
    using iterator = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    static constexpr size_type npos = size_type(-1);

    constexpr basic_string_view() noexcept : data_{nullptr}, size_{0}
    {
        // nothing
    }

    constexpr basic_string_view(const basic_string_view&) noexcept = default;
    basic_string_view& operator=(const basic_string_view&) noexcept = default;

    template <class Allocator>
    basic_string_view(
        const std::basic_string<Char, Traits, Allocator>& str) noexcept
        : data_{str.data()},
          size_{str.size()}
    {
        // nothing
    }

    constexpr basic_string_view(const Char* str)
        : data_{str}, size_{Traits::length(str)}
    {
        // nothing
    }

    constexpr basic_string_view(const Char* str, size_type len)
        : data_{str}, size_{len}
    {
        // nothing
    }

    constexpr const_iterator begin() const noexcept
    {
        return data_;
    }

    constexpr const_iterator end() const noexcept
    {
        return data_ + size_;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return {end()};
    }

    const_reverse_iterator rend() const noexcept
    {
        return {begin()};
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    constexpr size_type size() const noexcept
    {
        return size_;
    }

    constexpr size_type length() const noexcept
    {
        return size();
    }

    constexpr size_type max_size() const noexcept
    {
        return size();
    }

    constexpr bool empty() const noexcept
    {
        return size() == 0;
    }

    constexpr const_reference operator[](size_type pos) const
    {
        return data_[pos];
    }

    const_reference at(size_type pos) const
    {
        if (pos >= size())
            throw std::out_of_range{"index out of bounds"};
        return data_[pos];
    }

    constexpr const_reference front() const
    {
        return data_[0];
    }

    constexpr const_reference back() const
    {
        return data_[size_ - 1];
    }

    constexpr const_pointer data() const noexcept
    {
        return data_;
    }

    void clear() noexcept
    {
        data_ = nullptr;
        size_ = 0;
    }

    void remove_prefix(size_type n)
    {
        data_ += n;
        size_ -= n;
    }

    void remove_suffix(size_type n)
    {
        size_ -= n;
    }

    void swap(basic_string_view& s) noexcept
    {
        using ::std::swap;
        swap(data_, s.data_);
        swap(size_, s.size_);
    }

    template <class Allocator>
    explicit operator std::basic_string<Char, Traits, Allocator>() const
    {
        return {begin(), end()};
    }

    template <class Allocator = std::allocator<Char>>
    std::basic_string<Char, Traits, Allocator> to_string(const Allocator& a
                                                         = Allocator{}) const
    {
        return {begin(), end(), a};
    }

    size_type copy(Char* s, size_type n, size_type pos = 0) const
    {
        if (pos > size())
            throw std::out_of_range{"index out of bounds"};

        auto rlen = std::min(n, size() - pos);
        std::copy_n(begin() + pos, rlen, s);
        return rlen;
    }

    constexpr basic_string_view substr(size_type pos = 0,
                                       size_type n = npos) const
    {
        return pos > size()
                   ? throw std::out_of_range{"index out of bounds"}
                   : basic_string_view{data() + pos, std::min(n, size() - pos)};
    }

    int compare(basic_string_view s) const noexcept
    {
        auto cmp
            = Traits::compare(data(), s.data(), std::min(size(), s.size()));
        if (cmp != 0)
            return cmp;

        if (size() < s.size())
            return -1;

        if (size() == s.size())
            return 0;

        return 1;
    }

    constexpr int compare(size_type pos1, size_type n1,
                          basic_string_view s) const
    {
        return substr(pos1, n1).compare(s);
    }

    constexpr int compare(size_type pos1, size_type n1, basic_string_view s,
                          size_type pos2, size_type n2) const
    {
        return substr(pos1, n1).compare(s.substr(pos2, n2));
    }

    constexpr int compare(const Char* s) const
    {
        return compare(basic_string_view{s});
    }

    constexpr int compare(size_type pos1, size_type n1, const Char* s) const
    {
        return substr(pos1, n1).compare(basic_string_view{s});
    }

    constexpr int compare(size_type pos1, size_type n1, const Char* s,
                          size_type n2) const
    {
        return substr(pos1, n1).compare(basic_string_view{s, n2});
    }

    size_type find(basic_string_view s, size_type pos = 0) const noexcept
    {
        if (pos >= size())
            return npos;

        auto it
            = std::search(begin() + pos, end(), s.begin(), s.end(), Traits::eq);
        if (it == end())
            return npos;
        return std::distance(begin(), it);
    }

    constexpr size_type find(Char c, size_type pos = 0) const noexcept
    {
        return find(basic_string_view{&c, 1}, pos);
    }

    constexpr size_type find(const Char* s, size_type pos, size_type n) const
    {
        return find(basic_string_view{s, n}, pos);
    }

    constexpr size_type find(const Char* s, size_type pos = 0) const
    {
        return find(basic_string_view{s}, pos);
    }

    size_type rfind(basic_string_view s, size_type pos = npos) const noexcept
    {
        if (size() < s.size())
            return npos;

        pos = std::min(pos, size());
        if (s.size() < size() - pos)
            pos += s.size();
        else
            pos = size();

        auto it = std::find_end(begin(), begin() + pos, s.begin(), s.end(),
                                Traits::eq);

        if (it == begin() + pos)
            return npos;
        return std::distance(begin(), it);
    }

    constexpr size_type rfind(Char c, size_type pos = npos) const noexcept
    {
        return rfind(basic_string_view{&c, 1}, pos);
    }

    constexpr size_type rfind(const Char* s, size_type pos, size_type n) const
    {
        return rfind(basic_string_view{s, n}, pos);
    }

    constexpr size_type rfind(const Char* s, size_type pos = npos) const
    {
        return rfind(basic_string_view{s}, pos);
    }

    size_type find_first_of(basic_string_view s, size_type pos = 0) const
        noexcept
    {
        if (pos >= size())
            return npos;

        auto it = std::find_first_of(begin() + pos, end(), s.begin(), s.end(),
                                     Traits::eq);
        if (it == end())
            return npos;
        return std::distance(begin(), it);
    }

    constexpr size_type find_first_of(Char c, size_type pos = 0) const noexcept
    {
        return find_first_of(basic_string_view{&c, 1}, pos);
    }

    constexpr size_type find_first_of(const Char* s, size_type pos,
                                      size_type n) const
    {
        return find_first_of(basic_string_view{s, n}, pos);
    }

    constexpr size_type find_first_of(const Char* s, size_type pos = 0) const
    {
        return find_first_of(basic_string_view{s}, pos);
    }

    size_type find_last_of(basic_string_view s, size_type pos = npos) const
        noexcept
    {
        if (pos >= size())
            return npos;

        auto diff = size() - std::min(size(), pos);
        auto it = std::find_first_of(rbegin() + diff, rend(), s.begin(),
                                     s.end(), Traits::eq);
        if (it == rend())
            return npos;
        return size() - 1 - std::distance(rbegin(), it);
    }

    constexpr size_type find_last_of(Char c, size_type pos = npos) const
        noexcept
    {
        return find_last_of(basic_string_view{&c, 1}, pos);
    }

    constexpr size_type find_last_of(const Char* s, size_type pos,
                                     size_type n) const
    {
        return find_last_of(basic_string_view{s, n}, pos);
    }

    constexpr size_type find_last_of(const Char* s, size_type pos = npos) const
    {
        return find_last_of(basic_string_view{s}, pos);
    }

    size_type find_first_not_of(basic_string_view s, size_type pos = 0) const
        noexcept
    {
        if (pos >= size())
            return npos;

        auto it = std::find_if(begin(), end(), [&](const_reference c)
                               {
                                   return std::find(s.begin(), s.end(), c,
                                                    Traits::eq) == s.end();
                               });
        if (it == end())
            return npos;
        return std::distance(begin(), it);
    }

    constexpr size_type find_first_not_of(Char c, size_type pos = 0) const
        noexcept
    {
        return find_first_not_of(basic_string_view{&c, 1}, pos);
    }

    constexpr size_type find_first_not_of(const Char* s, size_type pos,
                                          size_type n) const
    {
        return find_first_not_of(basic_string_view{s, n}, pos);
    }

    constexpr size_type find_first_not_of(const Char* s,
                                          size_type pos = 0) const
    {
        return find_first_not_of(basic_string_view{s}, pos);
    }

    size_type find_last_not_of(basic_string_view s, size_type pos = npos) const
        noexcept
    {
        if (pos >= size())
            return npos;

        auto diff = size() - std::min(size(), pos);
        auto it = std::find_if(rbegin() + diff, rend(), [&](const_reference c)
                               {
                                   return std::find(s.begin(), s.end(), c,
                                                    Traits::eq) == s.end();
                               });
        if (it == rend())
            return npos;
        return size() - 1 - std::distance(rbegin(), it);
    }

    constexpr size_type find_last_not_of(Char c, size_type pos = npos) const
        noexcept
    {
        return find_last_not_of(basic_string_view{&c, 1}, pos);
    }

    constexpr size_type find_last_not_of(const Char* s, size_type pos,
                                         size_type n) const
    {
        return find_last_not_of(basic_string_view{s, n}, pos);
    }

    constexpr size_type find_last_not_of(const Char* s,
                                         size_type pos = npos) const
    {
        return find_last_not_of(basic_string_view{s}, pos);
    }

  private:
    const_pointer data_;
    size_type size_;
};

using string_view = basic_string_view<char>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;
using wstring_view = basic_string_view<wchar_t>;

namespace
{
template <class T>
using identity = typename std::decay<T>::type;
}

template <class Char, class Traits>
constexpr bool operator==(basic_string_view<Char, Traits> lhs,
                          basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

template <class Char, class Traits>
constexpr bool
    operator==(basic_string_view<Char, Traits> lhs,
               identity<basic_string_view<Char, Traits>> rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

template <class Char, class Traits>
constexpr bool operator==(identity<basic_string_view<Char, Traits>> lhs,
                          basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

template <class Char, class Traits>
constexpr bool operator!=(basic_string_view<Char, Traits> lhs,
                          basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

template <class Char, class Traits>
constexpr bool
    operator!=(basic_string_view<Char, Traits> lhs,
               identity<basic_string_view<Char, Traits>> rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

template <class Char, class Traits>
constexpr bool operator!=(identity<basic_string_view<Char, Traits>> lhs,
                          basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

template <class Char, class Traits>
constexpr bool operator<(basic_string_view<Char, Traits> lhs,
                         basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

template <class Char, class Traits>
constexpr bool operator<(basic_string_view<Char, Traits> lhs,
                         identity<basic_string_view<Char, Traits>> rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

template <class Char, class Traits>
constexpr bool operator<(identity<basic_string_view<Char, Traits>> lhs,
                         basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

template <class Char, class Traits>
constexpr bool operator>(basic_string_view<Char, Traits> lhs,
                         basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

template <class Char, class Traits>
constexpr bool operator>(basic_string_view<Char, Traits> lhs,
                         identity<basic_string_view<Char, Traits>> rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

template <class Char, class Traits>
constexpr bool operator>(identity<basic_string_view<Char, Traits>> lhs,
                         basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

template <class Char, class Traits>
constexpr bool operator<=(basic_string_view<Char, Traits> lhs,
                          basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

template <class Char, class Traits>
constexpr bool
    operator<=(basic_string_view<Char, Traits> lhs,
               identity<basic_string_view<Char, Traits>> rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

template <class Char, class Traits>
constexpr bool operator<=(identity<basic_string_view<Char, Traits>> lhs,
                          basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

template <class Char, class Traits>
constexpr bool operator>=(basic_string_view<Char, Traits> lhs,
                          basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}

template <class Char, class Traits>
constexpr bool
    operator>=(basic_string_view<Char, Traits> lhs,
               identity<basic_string_view<Char, Traits>> rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}

template <class Char, class Traits>
constexpr bool operator>=(identity<basic_string_view<Char, Traits>> lhs,
                          basic_string_view<Char, Traits> rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}

template <class Char, class Traits>
std::basic_ostream<Char, Traits>&
    operator<<(std::basic_ostream<Char, Traits>& os,
               basic_string_view<Char, Traits> str)
{
    return os << str.to_string();
}
}
}

namespace std
{
template <class Char, class Traits>
struct hash<meta::util::basic_string_view<Char, Traits>>
    : public meta::util::hash<>
{
};
}
#endif // !META_HAS_EXPERIMENTAL_STRING_VIEW

namespace meta
{
namespace util
{
template <class HashAlgorithm, class Char, class Traits>
typename std::enable_if<is_contiguously_hashable<Char>::value>::type
    hash_append(HashAlgorithm& h, const basic_string_view<Char, Traits>& s)
{
    h(s.data(), s.size() * sizeof(Char));
    hash_append(h, s.size());
}

template <class HashAlgorithm, class Char, class Traits>
typename std::enable_if<!is_contiguously_hashable<Char>::value>::type
    hash_append(HashAlgorithm& h, const basic_string_view<Char, Traits>& s)
{
    for (const auto& c : s)
        hash_append(h, c);
    hash_append(h, s.size());
}
}
}
#endif // META_UTIL_STRING_VIEW_H_
