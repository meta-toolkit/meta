/**
 * @file splay_cache.tcc
 */

#include "caching/splay_cache.h"

namespace meta {
namespace caching {

template <class Key, class Value>
splay_cache<Key, Value>::splay_cache(uint64_t max_size):
    _size{0}, _max_size(max_size), _root(nullptr)
{ /* nothing */ }

template <class Key, class Value>
splay_cache<Key, Value>::splay_cache(splay_cache && other):
    _size{std::move(other._size)},
    _max_size{std::move(other._max_size)},
    _root{std::move(other._root)}
{ /* nothing */ }

template <class Key, class Value>
splay_cache<Key, Value> &
splay_cache<Key, Value>::operator=(splay_cache && rhs)
{
    if(this != &rhs)
    {
        _size = std::move(rhs._size);
        _max_size = std::move(rhs._max_size);
        _root = std::move(rhs._root);
    }
    return *this;
}

template <class Key, class Value>
splay_cache<Key, Value>::~splay_cache()
{
    clear(_root);
}

template <class Key, class Value>
void splay_cache<Key, Value>::insert(const Key & key, const Value & value)
{
    std::lock_guard<std::mutex> lock{_mutables};
    insert(_root, key, value);
}

template <class Key, class Value>
void splay_cache<Key, Value>::insert(node* & subroot, const Key & key,
                                     const Value & value)
{
    if(subroot == nullptr)
    {
        subroot = new node{key, value};
        ++_size;
    }
    else if(key < subroot->key)
    {
        if(_size == _max_size && !subroot->left)
        {
            replace(subroot, key, value);
        }
        else
        {
            insert(subroot->left, key, value);
            rotate_right(subroot);
        }
    }
    else if(key > subroot->key)
    {
        if(_size == _max_size && !subroot->right)
        {
            replace(subroot, key, value);
        }
        else
        {
            insert(subroot->right, key, value);
            rotate_left(subroot);
        }
    }
}

template <class Key, class Value>
void splay_cache<Key, Value>::replace(node* subroot, const Key & key,
                                      const Value & value)
{
    for (auto & callback : _drop_callbacks)
        callback(subroot->key, subroot->value);
    subroot->key = key;
    subroot->value = value;
}

template <class Key, class Value>
util::optional<Value> splay_cache<Key, Value>::find(const Key & key)
{
    std::lock_guard<std::mutex> lock{_mutables};
    if(_root != nullptr)
    {
        find(_root, key);
        if(_root->key == key)
            return {_root->value};
    }
    return {util::nullopt};
}

template <class Key, class Value>
void splay_cache<Key, Value>::find(node* & subroot, const Key & key)
{
    if(subroot == nullptr)
        return;

    if(key < subroot->key)
    {
        find(subroot->left, key);
        if(subroot->left != nullptr)
            rotate_right(subroot);
    }
    else if(key > subroot->key)
    {
        find(subroot->right, key);
        if(subroot->right != nullptr)
            rotate_left(subroot);
    }
}

template <class Key, class Value>
void splay_cache<Key, Value>::clear(node* & subroot)
{
    if(subroot != nullptr)
    {
        clear(subroot->left);
        clear(subroot->right);
        for (auto & callback : _drop_callbacks)
            callback(subroot->key, subroot->value);
        delete subroot;
        subroot = nullptr;
    }
}

template <class Key, class Value>
void splay_cache<Key, Value>::rotate_left(node* & subroot)
{
    node* new_subroot = subroot->right;
    node* middle = new_subroot->left;

    new_subroot->left = subroot;
    subroot->right = middle;
    subroot = new_subroot;
}

template <class Key, class Value>
void splay_cache<Key, Value>::rotate_right(node* & subroot)
{

    node* new_subroot = subroot->left;
    node* middle = new_subroot->right;

    subroot->left = middle;
    new_subroot->right = subroot;
    subroot = new_subroot;
}

template <class Key, class Value>
uint64_t splay_cache<Key, Value>::size() const
{
    return _size;
}

template <class Key, class Value>
template <class Functor>
void splay_cache<Key, Value>::on_drop(Functor && fun) {
    _drop_callbacks.emplace_back(std::forward<Functor>(fun));
}

template <class Key, class Value>
void splay_cache<Key, Value>::clear() {
    clear(_root);
}

}
}
