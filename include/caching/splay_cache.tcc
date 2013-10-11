/**
 * @file splay_cache.tcc
 */

#include "caching/splay_cache.h"

namespace meta {
namespace caching {

template <class Key, class Value>
splay_cache<Key, Value>::splay_cache(uint32_t max_height):
    _max_height(max_height), _root(nullptr)
{
    if(_max_height < 2)
        throw splay_cache_exception{"max height must be greater than 1"};
}

template <class Key, class Value>
splay_cache<Key, Value>::splay_cache(splay_cache && other)
    : _max_height{std::move(other._max_height)}, _root{std::move(other._root)}
{ /* nothing */ }

template <class Key, class Value>
splay_cache<Key, Value> &
splay_cache<Key, Value>::operator=(splay_cache && rhs)
{
    if(this != &rhs) {
        _max_height = std::move(rhs._max_height);
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
    insert(_root, key, value, 0);
}

template <class Key, class Value>
void splay_cache<Key, Value>::insert(node* & subroot, const Key & key,
                                     const Value & value, uint32_t depth)
{
    if(subroot == nullptr)
        subroot = new node{key, value};
    else if(key < subroot->key)
    {
        insert(subroot->left, key, value, depth + 1);
        rotate_right(subroot, depth);
    }
    else if(key > subroot->key)
    {
        insert(subroot->right, key, value, depth + 1);
        rotate_left(subroot, depth);
    }
}

template <class Key, class Value>
util::optional<Value> splay_cache<Key, Value>::find(const Key & key)
{
    std::lock_guard<std::mutex> lock{_mutables};
    if(_root != nullptr)
    {
        find(_root, key, 0);
        if(_root->key == key)
            return {_root->value};
    }
    return {util::nullopt};
}

template <class Key, class Value>
void splay_cache<Key, Value>::find(node* & subroot,
        const Key & key, uint32_t depth)
{
    if(subroot == nullptr)
        return;

    if(key < subroot->key)
    {
        find(subroot->left, key, depth + 1);
        if(subroot->left != nullptr)
            rotate_right(subroot, depth);
    }
    else if(key > subroot->key)
    {
        find(subroot->right, key, depth + 1);
        if(subroot->right != nullptr)
            rotate_left(subroot, depth);
    }
}

template <class Key, class Value>
void splay_cache<Key, Value>::clear(node* & subroot)
{
    if(subroot != nullptr)
    {
        clear(subroot->left);
        clear(subroot->right);
        delete subroot;
        subroot = nullptr;
    }
}

template <class Key, class Value>
void splay_cache<Key, Value>::rotate_left(node* & subroot, uint32_t depth)
{
    node* new_subroot = subroot->right;
    node* middle = new_subroot->left;

    new_subroot->left = subroot;
    subroot->right = middle;
    subroot = new_subroot;

    // when rotating left, the depth of new_subroot->left->left's children could
    // increase the height of the tree if they exist
    if(depth + 3 >= _max_height && new_subroot->left && new_subroot->left->left)
    {
        clear(new_subroot->left->left->left);
        clear(new_subroot->left->left->right);
    }
}

template <class Key, class Value>
void splay_cache<Key, Value>::rotate_right(node* & subroot, uint32_t depth)
{

    node* new_subroot = subroot->left;
    node* middle = new_subroot->right;

    subroot->left = middle;
    new_subroot->right = subroot;
    subroot = new_subroot;

    // when rotating right, the depth of new_subroot->right->right's children
    // could increase the height of the tree if they exist
    if(depth + 3 >= _max_height
            && new_subroot->right && new_subroot->right->right)
    {
        clear(new_subroot->right->right->left);
        clear(new_subroot->right->right->right);
    }
}

}
}
