/**
 * @file progress.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_PROGRESS_H_
#define META_UTIL_PROGRESS_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "meta/config.h"

namespace meta
{
namespace printing
{

/**
 * Simple class for reporting progress of lengthy operations. Inspired by
 * noamraph/tqdm.
 *
 * @see https://github.com/noamraph/tqdm/
 */
class progress
{
  public:
    /**
     * Constructs a progress reporter with the given prefix and iteration
     * length.
     *
     * @param prefix The string to be printed right before the progress
     * output
     * @param length The number of iterations
     * @param interval The length of time, in milliseconds, to wait
     * between updates. Default = 500ms.
     */
    progress(const std::string& prefix, uint64_t length, int interval = 500);

    /**
     * Sets whether or not an endline should be printed at completion.
     * @param endline Whether or not an endline should be printed at
     * completion
     */
    void print_endline(bool endline);

    /**
     * Destroys this progress reporter. It will call end() if it has not
     * already been called.
     */
    ~progress();

    /**
     * Updates the progress indicator. Since progress is printed
     * asynchronously, you may not immediately see results after calling
     * this function, but they will be reflected in the next update tick.
     * @param iter The current iteration number to update to
     */
    void operator()(uint64_t iter);

    /**
     * Marks the progress indicator as having finished.
     */
    void end();

    /**
     * Clears the last line the progress bar wrote.
     */
    void clear() const;

  private:
    void print();
    void progress_thread();

    /// The background thread for printing progress updates
    std::thread thread_;
    /// The mutex for the condition variable.
    std::mutex mutex_;
    /// The condition variable used by the background thread for sleeping
    std::condition_variable cond_var_;
    /// The output line
    std::string output_;
    /// The length of the prefix
    const std::size_t prefix_len_;
    /// The start time of the job.
    const std::chrono::steady_clock::time_point start_;
    /// The current iteration number.
    std::atomic<uint64_t> iter_;
    ///  The total number of iterations.
    const uint64_t length_;
    /// The length of time, in milliseconds, to wait between updates.
    const int interval_;
    /// Whether or not we should print an endline when done.
    bool endline_;
};
}
}
#endif
