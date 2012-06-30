/**
 * @file invertible_map.h
 */

#ifndef _INVERTIBLE_MAP_H_
#define _INVERTIBLE_MAP_H_

#include <iterator>
#include <utility>
#include <iostream>
#include <map>
#include <unordered_map>

using std::pair;
using std::make_pair;
using std::map;
using std::unordered_map;
using std::cerr;
using std::endl;

typedef Iterator iterator;
typedef Iterator const_iterator;
typedef RevIterator reverse_iterator;
typedef RevIterator const_reverse_iterator;
typedef InnerIterator unordered_map<Key, Value>::iterator;
typedef InnerIterator unordered_map<Key, Value>::const_iterator;
typedef RevInnerIterator unordered_map<Key, Value>::reverse_iterator;
typedef RevInnerIterator unordered_map<Key, Value>::const_reverse_iterator;

/**
 * This data structure indexes by keys as well as values, allowing constant
 *  amortized lookup time by key or value. All keys and values must be unique.
 */
template <class Key, class Value>
class InvertibleMap
{
    public:

        /**
         * Constructor.
         */
        InvertibleMap():
            _forward(unordered_map<Key, Value>()),
            _backward(unordered_map<Value, Key>()) { /* nothing */ }

        /**
         * @return whether the invertible map is empty
         */
        bool empty() const;

        /**
         * @return the number of elements in the invertible map
         */
        size_t size() const;

        /**
         * @return a key given a value
         */
        Key getKeyByValue(const Value & value) const;

        /**
         * @return a value given a key
         */
        Value getValueByKey(const Key & key) const;

        /**
         * Inserts a (key, value) pair into the invertible map
         */
        void insert(const Key & key, const Value & value);

        /**
         * @return a (key, value) map sorted by keys
         */
        map<Key, Value> sortKeys() const;

        /**
         * @return a (value, key) map sorted by values
         */
        map<Value, Key> sortValues() const;
 
        class Iterator: public std::iterator<std::bidirectional_iterator_tag, unordered_map<Key, Value>::iterator iter>
        {
            public:

                Iterator(): iter(InnerIterator()) { }

                Iterator(const InnerIterator & it): iter(it) { }
                
                // Pre-Increment
                Iterator & operator++(){
                    ++iter;
                    return *this;
                }

                // Post-increment
                Iterator operator++(int){
                    InnerIter r = iter;
                    ++iter;
                    return Iterator(r);
                }

                // Pre-decrement
                Iterator & operator--(){
                    --iter;
                    return *this;
                }

                // Post-decrement
                Iterator operator--(int)
                    InnerIter r = iter;
                    --iter;
                    return Iterator(r);
                }

                bool operator==(const Iterator & other){
                    return iter == other.iter;
                }

                bool operator!=(const Iterator & other){
                    return iter != other.iter;
                }

                const InnerIterator & operator*(){
                    return *iter;
                }

                const InnerIterator * operator->(){
                    return &(*iter);
                }

            private:
                InnerIterator iter;
        };

/*
        class RevIterator : public std::iterator<std::bidirectional_iterator_tag, pair<Key, Value> >
        {
            public:
                RevIterator(): p(NULL) { }
                RevIterator(ListNode * x) : p(x) { }
                RevIterator& operator++()   { p = p->prev; return *this; } // Pre-Increment
                RevIterator operator++(int) { ListNode* r = p; p = p->prev; return RevIterator(r); } // Post-Increment
                RevIterator& operator--()   { p = p->next; return *this; } // Pre-Decrement
                RevIterator operator--(int) { ListNode* r = p; p = p->next; return RevIterator(r); } // Post-Decrement
                bool operator==(const RevIterator& rhs) { return p == rhs.p; }
                bool operator!=(const RevIterator& rhs) { return p != rhs.p; }
                const T & operator*() { return p->data; }
                const T * operator->() { return &(p->data); }

            private:
        };
*/
        const_iterator begin() const { return Iterator(head); }
        const_iterator end() const { return Iterator(NULL); }
        //const_reverse_iterator rbegin() const { return RevIterator(tail); }
        //const_reverse_iterator rend() const { return RevIterator(NULL); }

        // Iterator constructor
        template <class Iter>
        InvertibleMap(const Iter & start, const Iter & end);

    private:

        unordered_map<Key, Value> _forward;
        unordered_map<Value, Key> _backward;
};

#include "invertible_map.cpp"
#endif
