/**
 * @file parallel_for.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DST_PARALLEL_FOR_H_
#define _DST_PARALLEL_FOR_H_

#include <algorithm>
#include <iterator>
#include <thread>
#include <vector>

#include "parallel/thread_pool.h"

namespace meta {
namespace parallel {
    
template <class Iterator, class Function>
void parallel_for( Iterator begin, Iterator end, Function func ) {
    thread_pool pool;
    parallel_for( begin, end, pool, func );
}

template <class Iterator, class Function>
void parallel_for( Iterator begin, Iterator end, thread_pool & pool, Function func ) {
    auto block_size = std::distance( begin, end ) / std::thread::hardware_concurrency();

    Iterator last = begin;
    if( block_size > 0 ) {
        std::advance( last, ( std::thread::hardware_concurrency() - 1 ) * block_size );
    } else {
        last = end;
        block_size = 1;
    }

    std::vector<std::future<void>> futures;
    // first p - 1 groups
    for( ; begin != last; std::advance( begin, block_size ) ) {
        futures.emplace_back( 
                pool.submit_task(
                    [=]() {
                        auto mylast = begin;
                        std::advance( mylast, block_size );
                        std::for_each( begin, mylast, func );
                    }
                )
            );
    }
    // last group
    futures.emplace_back(
            pool.submit_task( [=]() {
                    std::for_each( begin, end, func );
                }
            )
        );
    for( auto & fut : futures )
        fut.get();
}

}
}

#endif
