/**
 * @file splay_cache.tcc
 */

#include "meta/caching/splay_cache.h"

namespace meta
{
namespace caching
{

template <class Key, class Value>
splay_cache<Key, Value>::splay_cache(uint64_t max_size)
    : size_{0}, max_size_(max_size), root_(nullptr)
{
    /* nothing */
}

template <class Key, class Value>
splay_cache<Key, Value>::splay_cache(splay_cache&& other)
    : size_{std::move(other.size_)},
      max_size_{std::move(other.max_size_)},
      root_{std::move(other.root_)}
{
    /* nothing */
}

template <class Key, class Value>
splay_cache<Key, Value>& splay_cache<Key, Value>::operator=(splay_cache&& rhs)
{
    if (this != &rhs)
    {
        size_ = std::move(rhs.size_);
        max_size_ = std::move(rhs.max_size_);
        root_ = std::move(rhs.root_);
    }
    return *this;
}

template <class Key, class Value>
splay_cache<Key, Value>::~splay_cache()
{
    clear(root_);
}

template <class Key, class Value>
void splay_cache<Key, Value>::insert(const Key& key, const Value& value)
{
    std::lock_guard<std::mutex> lock{mutables_};
    insert(root_, key, value);
}

template <class Key, class Value>
void splay_cache
    <Key, Value>::insert(node*& subroot, const Key& key, const Value& value)
{
    if (subroot == nullptr)
    {
        subroot = new node{key, value};
        ++size_;
    }
    else if (key < subroot->key)
    {
        if (size_ == max_size_ && !subroot->left)
        {
            replace(subroot, key, value);
        }
        else
        {
            insert(subroot->left, key, value);
            rotate_right(subroot);
        }
    }
    else if (key > subroot->key)
    {
        if (size_ == max_size_ && !subroot->right)
        {
            replace(subroot, key, value);
        }
        else
        {
            insert(subroot->right, key, value);
            rotate_left(subroot);
        }
    }
    else if (key == subroot->key)
    {
        subroot->value = value;
    }
}

template <class Key, class Value>
void splay_cache
    <Key, Value>::replace(node* subroot, const Key& key, const Value& value)
{
    subroot->key = key;
    subroot->value = value;
}

template <class Key, class Value>
util::optional<Value> splay_cache<Key, Value>::find(const Key& key)
{
    std::lock_guard<std::mutex> lock{mutables_};
    if (root_ != nullptr)
    {
        find(root_, key);
        if (root_->key == key)
            return {root_->value};
    }
    return {util::nullopt};
}

template <class Key, class Value>
void splay_cache<Key, Value>::find(node*& subroot, const Key& key)
{
    if (subroot == nullptr)
        return;

    if (key < subroot->key)
    {
        find(subroot->left, key);
        if (subroot->left != nullptr)
            rotate_right(subroot);
    }
    else if (key > subroot->key)
    {
        find(subroot->right, key);
        if (subroot->right != nullptr)
            rotate_left(subroot);
    }
}

template <class Key, class Value>
void splay_cache<Key, Value>::clear(node*& subroot)
{
    if (subroot != nullptr)
    {
        clear(subroot->left);
        clear(subroot->right);
        delete subroot;
        subroot = nullptr;
    }
}

template <class Key, class Value>
void splay_cache<Key, Value>::rotate_left(node*& subroot)
{
    node* new_subroot = subroot->right;
    node* middle = new_subroot->left;

    new_subroot->left = subroot;
    subroot->right = middle;
    subroot = new_subroot;
}

template <class Key, class Value>
void splay_cache<Key, Value>::rotate_right(node*& subroot)
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
    return size_;
}

template <class Key, class Value>
void splay_cache<Key, Value>::clear()
{
    std::lock_guard<std::mutex> lock{mutables_};
    clear(root_);
}
}
}
