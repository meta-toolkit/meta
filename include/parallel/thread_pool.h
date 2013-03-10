/**
 * @file thread_pool.h
 * Definition of a thread pool class that utilizes C++11 threading
 * facilities.
 * 
 * @see Inspired by: https://github.com/progschj/ThreadPool
 */

#ifndef _DST_THREAD_POOL_H_
#define _DST_THREAD_POOL_H_

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <future>

namespace parallel {

class thread_pool {
    public:
        thread_pool( size_t num_threads = std::thread::hardware_concurrency() )
                : running_( true ) {
            for( size_t i = 0; i < num_threads; ++i )
                threads_.push_back( 
                        std::thread{ std::bind( &thread_pool::worker, this ) } 
                    );
        }

        ~thread_pool() {
            {
                std::unique_lock<std::mutex> lock( mutex_ );
                running_ = false;
            }
            cond_.notify_all();
            for( auto & thread : threads_ )
                thread.join();
        }

        template <class Function>
        std::future<typename std::result_of<Function()>::type>
        submit_task( Function func ) {
            using result_type = typename std::result_of<Function()>::type;

            std::unique_ptr<concrete_task<result_type>> task( 
                    new concrete_task<result_type>( func ) );

            auto future = task->get_future();
            {
                std::unique_lock<std::mutex> lock( mutex_ );
                tasks_.push( std::move( task ) );
            }
            cond_.notify_one();
            return future;
        }

    private:

        struct task {
            virtual void run() = 0;
        };

        template <class R>
        struct concrete_task : task { 
            template <class Function>
            concrete_task( const Function & f )
                    : task_( f ) {
            }

            virtual void run() {
                task_();
            }

            std::future<R> get_future() {
                return task_.get_future();
            }

            std::packaged_task<R()> task_;
        };

        void worker() {
            while( true ) {
                std::unique_ptr<task> task;
                {
                    std::unique_lock<std::mutex> lock( mutex_ );
                    while( running_ && tasks_.empty() )
                        cond_.wait( lock );
                    if( !running_ && tasks_.empty() )
                        return;
                    task = std::move( tasks_.front() );
                    tasks_.pop();
                }
                task->run();
            }
        }

        std::vector<std::thread> threads_;
        std::queue<std::unique_ptr<task>> tasks_;

        bool running_;

        std::mutex mutex_;
        std::condition_variable cond_;
};

}
#endif
