/**
 * @file parallel_for.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_PARALLEL_FOR_H_
#define META_PARALLEL_FOR_H_

#include <algorithm>
#include <iterator>
#include <thread>
#include <vector>

#include "meta/config.h"
#include "meta/parallel/thread_pool.h"

namespace meta
{
namespace parallel
{

/**
 * Runs the given function on sub-ranges of [begin, end) in parallel.
 * @param begin The beginning of the range
 * @param end The ending of the range
 * @param pool The thread_pool to run on
 * @param fn The binary function that operates over iterator ranges
 */
template <class Iterator, class Function>
std::vector<std::future<
    typename std::result_of<Function(Iterator, Iterator)>::type>>
for_each_block(Iterator begin, Iterator end, thread_pool& pool, Function&& fn)
{
    using difference_type =
        typename std::iterator_traits<Iterator>::difference_type;
    using result_type =
        typename std::result_of<Function(Iterator, Iterator)>::type;

    auto pool_size = static_cast<difference_type>(pool.size());
    auto block_size = std::distance(begin, end) / pool_size;

    Iterator last = begin;
    if (block_size > 0)
    {
        std::advance(last, (pool_size - 1) * block_size);
    }
    else
    {
        last = end;
        block_size = 1;
    }

    std::vector<std::future<result_type>> futures;
    // first p - 1 groups
    for (; begin != last; std::advance(begin, block_size))
    {
        futures.emplace_back(pool.submit_task([=]() {
            auto mylast = begin;
            std::advance(mylast, block_size);
            return fn(begin, mylast);
        }));
    }
    // last group
    futures.emplace_back(pool.submit_task([=]() { return fn(begin, end); }));

    return futures;
}

/**
 * Runs the given function on the range denoted by begin and end in parallel.
 * @param begin The first element to operate on
 * @param end One past the last element to operate on
 * @param func The function to perform on each element
 */
template <class Iterator, class Function>
void parallel_for(Iterator begin, Iterator end, Function func)
{
    thread_pool pool;
    parallel_for(begin, end, pool, func);
}

/**
 * Runs the given function on the range denoted by begin and end in parallel.
 * @param begin The first element to operate on
 * @param end One past the last element to operate on
 * @param pool The thread pool to use
 * @param func The function to perform on each element
 */
template <class Iterator, class Function>
void parallel_for(Iterator begin, Iterator end, thread_pool& pool,
                  Function func)
{
    auto futures
        = for_each_block(begin, end, pool, [&](Iterator tbegin, Iterator tend) {
              std::for_each(tbegin, tend, func);
          });
    for (auto& fut : futures)
        fut.get();
}
}
}

#endif
