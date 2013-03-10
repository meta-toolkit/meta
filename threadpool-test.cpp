#include <chrono>
#include <vector>
#include <iostream>
#include "parallel/thread_pool.h"

int main() {
    using namespace parallel;
    thread_pool pool{};

    std::vector<std::future<size_t>> futures;

    for( size_t i = 0; i < 16; ++i ) {
        futures.push_back(
                pool.submit_task([i]() {
                    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
                    return i * i;
                })
            );
    }

    for( auto & fut : futures )
        std::cout << fut.get() << ' ';
    std::cout << std::endl;
}
