/**
 * @file range.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DST_UTIL_RANGE_H_
#define _DST_UTIL_RANGE_H_

#include <cmath>
#include <iterator>

namespace meta {
namespace util {

template <class T>
class basic_range { 
    public:
        template <class Plus>
        class iterator_t {
            public:
                typedef iterator_t                  self_type;
                typedef T                           value_type;
                typedef const T &                   reference;
                typedef const T *                   pointer;
                typedef std::forward_iterator_tag   iterator_category;
                typedef std::ptrdiff_t              difference_type;
                
                friend class basic_range<T>;
                
                /**
                 * Prefix increment.
                 */
                self_type & operator++() {
                    _curr = _plus( _curr, _step );
                    ++_idx;
                    return *this;
                }
                
                /**
                 * Postfix increment.
                 */
                self_type operator++(int) {
                    self_type temp = *this;
                    ++(*this);
                    return temp;
                }
                
                /**
                 * Dereference operator.
                 */
                reference operator*() {
                    return _curr;
                }
                
                /**
                 * Member access operator.
                 */
                pointer operator->() {
                    return &_curr;
                }
                
                /**
                 * Equality operator.
                 */
                friend bool operator==( const iterator_t & lhs, const iterator_t & rhs ) {
                    return lhs._range == rhs._range && lhs._idx == rhs._idx && lhs._step == rhs._step;
                }
                
                /**
                 * Inequality operator.
                 */
                friend bool operator!=( const iterator_t & lhs, const iterator_t & rhs ) {
                    return !(lhs == rhs);
                }
            private:
                iterator_t( const basic_range<T> * range, const T & start, size_t idx, std::ptrdiff_t step ) :
                    _curr( start ), _idx( idx ), _range( range ), _step( step ) {
                    // nothing
                }
                
                T _curr;
                size_t _idx;
                Plus _plus;
                const basic_range<T> * _range;
                std::ptrdiff_t _step;
        };
        typedef iterator_t<std::plus<T>> iterator;
        typedef iterator const_iterator;
        
        typedef iterator_t<std::minus<T>> reverse_iterator;
        typedef reverse_iterator const_reverse_iterator;
        
        friend iterator;
        friend reverse_iterator;
        
        iterator begin() const {
            return iterator( this, _begin, 0, _step );
        }
        
        iterator end() const {
            return iterator( this, _end, _num, _step );
        }
        
        reverse_iterator rbegin() const {
            return reverse_iterator( this, _end, 0, _step );
        }
        
        reverse_iterator rend() const {
            return reverse_iterator( this, _begin, _num, _step );
        }
        
        basic_range( const T & begin, const T & end, std::ptrdiff_t step ) 
                : _begin( begin ), _end( end ), 
                  _num( ( end - begin ) / step + 1 ),
                  _step( step ) {
            // nothing
        }

        ~basic_range() = default;
    private:
        T _begin;
        T _end;
        size_t _num;
        std::ptrdiff_t _step;
};

template <class T>
basic_range<T> range( const T & begin, const T & end, std::ptrdiff_t step = 1 ) {
    return basic_range<T>( begin, end, step );
}

}
}

#endif
