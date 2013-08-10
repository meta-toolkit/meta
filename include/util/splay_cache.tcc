/**
 * @file splay_cache.tcc
 */

namespace meta {
namespace util {

template <class Key, class Value>
splay_cache<Key, Value>::splay_cache(uint32_t max_height):
    _max_height(max_height), _root(nullptr)
{
    if(_max_height < 1)
        throw splay_cache_exception{"max height must be greater than 0"};
}

template <class Key, class Value>
splay_cache<Key, Value>::~splay_cache()
{
    clear(_root);
}

template <class Key, class Value>
void splay_cache<Key, Value>::insert(const Key & key, const Value & value)
{
    insert(_root, key, value, 1);
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
        rotate_right(subroot);

        // see if the tree is too tall; if so, remove the last leaf node (on
        // opposite side now since we rotated)
        if(depth > _max_height)
        {
            delete subroot->left;
            subroot->left = nullptr;
        }
    }
    else if(key > subroot->key)
    {
        insert(subroot->right, key, value, depth + 1);
        rotate_left(subroot);

        // see if the tree is too tall; opposite case as above
        if(depth > _max_height)
        {
            delete subroot->right;
            subroot->right = nullptr;
        }
    }
}

template <class Key, class Value>
bool splay_cache<Key, Value>::exists(const Key & key)
{
    if(_root != nullptr)
    {
        find(_root, key);
        return _root->key == key;
    }

    return false;
}

template <class Key, class Value>
const Value & splay_cache<Key, Value>::find(const Key & key)
{
    if(_root == nullptr)
        throw splay_cache_exception{"find called on empty cache; call exists first"};

    find(_root, key);
    return _root->value;
}

template <class Key, class Value>
void splay_cache<Key, Value>::find(node* & subroot, const Key & key)
{
    if(subroot == nullptr)
        return;

    if(key < subroot->key)
    {
        find(subroot->left, key);
        rotate_right(subroot);
    }
    else if(key > subroot->key)
    {
        find(subroot->right, key);
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

}
}
