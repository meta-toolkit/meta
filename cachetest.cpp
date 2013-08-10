/**
 * @file cache-test.cpp
 */

#include <iostream>
#include "util/splay_cache.h"

using namespace meta;

int main()
{
    util::splay_cache<int, int> cache{1};

    cache.insert(5, 7);
    cache.insert(89, 123);
    cache.insert(86, 123);
    cache.insert(8, 123);

    std::cout << (cache.find(5) == 7) << std::endl;

    return 0;
};
