/**
 * @file thread_pool.h
 * @author Chase Geigle
 * @see Inspired by: https://github.com/progschj/ThreadPool
 * Definition of a thread pool class that utilizes C++11 threading
 * facilities.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_THREAD_POOL_H_
#define META_THREAD_POOL_H_

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <future>

namespace meta
{
namespace parallel
{

/**
 * Represents a collection of a fixed number of threads, which tasks can be
 * added to.
 */
class thread_pool
{
  public:
    /**
     * @param num_threads The number of threads to initialize this thread_pool
     * with; by default, the hardware concurrency.
     */
    thread_pool(size_t num_threads = std::thread::hardware_concurrency())
        : running_(true)
    {
        for (size_t i = 0; i < num_threads; ++i)
            threads_.push_back(
                std::thread{std::bind(&thread_pool::worker, this)});
    }

    /**
     * Destructor; joins all threads.
     */
    ~thread_pool()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            running_ = false;
        }
        cond_.notify_all();
        for (auto& thread : threads_)
            thread.join();
    }

    /**
     * Adds a task to the thread_pool.
     * @param func The function (task) to add
     */
    template <class Function>
    std::future<typename std::result_of<Function()>::type>
    submit_task(Function func)
    {
        using result_type = typename std::result_of<Function()>::type;

        std::unique_ptr<concrete_task<result_type>> task(
            new concrete_task<result_type>(func));

        auto future = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }
        cond_.notify_one();
        return future;
    }

    /**
     * @return a vector of the thread_ids from the current pool
     */
    std::vector<std::thread::id> thread_ids() const
    {
        std::vector<std::thread::id> ids;
        for (auto& t : threads_)
            ids.emplace_back(t.get_id());
        return ids;
    }

  private:
    struct task
    {
        virtual void run() = 0;
        virtual ~task()
        {
        } // silence g++ errors
    };

    template <class R>
    struct concrete_task : task
    {
        template <class Function>
        concrete_task(const Function& f)
            : task_(f)
        {
        }

        virtual ~concrete_task()
        {
        } // silence g++ errors

        virtual void run()
        {
            task_();
        }

        std::future<R> get_future()
        {
            return task_.get_future();
        }

        std::packaged_task<R()> task_;
    };

    void worker()
    {
        while (true)
        {
            std::unique_ptr<task> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while (running_ && tasks_.empty())
                    cond_.wait(lock);
                if (!running_ && tasks_.empty())
                    return;
                task = std::move(tasks_.front());
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
}

#endif
