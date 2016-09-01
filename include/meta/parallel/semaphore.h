/**
 * @file semaphore.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For
 * more details, consult the file LICENSE.mit and LICENSE.ncsa in the root
 * of the project.
 */

#ifndef META_PARALLEL_MUTEX_H_
#define META_PARALLEL_MUTEX_H_

#include <condition_variable>
#include <mutex>

#include "meta/config.h"

namespace meta
{
namespace parallel
{

/**
 * Implements a counting semaphore. Threads are allowed to continue into
 * the critical section if the count is positive. If it is not, they must
 * wait until the count becomes positive again.
 */
class semaphore
{
  public:
    /**
     * Constructs the semaphore to allow count number of threads at a time.
     */
    semaphore(unsigned count) : count_{count}
    {
        // nothing
    }

    /**
     * RAII class for waiting and signaling on the semaphore. Its
     * constructor will wait, and its destructor will signal; thus,
     * construction will result in waiting until the count is positive.
     */
    class wait_guard
    {
      public:
        wait_guard(semaphore& sem) : sem_(sem)
        {
            std::unique_lock<std::mutex> lock{sem_.mutex_};
            while (!sem_.count_)
                sem_.cond_.wait(lock);
            --sem_.count_;
        }

        ~wait_guard()
        {
            {
                std::unique_lock<std::mutex> lock{sem_.mutex_};
                ++sem_.count_;
            }
            sem_.cond_.notify_one();
        }

      private:
        semaphore& sem_;
    };
    friend wait_guard;

  private:
    unsigned count_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
}
}
#endif
