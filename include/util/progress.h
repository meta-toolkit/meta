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

#include <chrono>
#include <string>

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
     * @param min_iters The minimum number of iterations that must pass
     * before progress reporting will be considered
     */
    progress(const std::string& prefix, uint64_t length,
             int interval = 500, uint64_t min_iters = 10);

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
     * Updates the progress indicator.
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
    /// The prefix for the progress report message.
    std::string prefix_;
    /// The start time of the job.
    std::chrono::steady_clock::time_point start_;
    /// The time of the last update.
    std::chrono::steady_clock::time_point last_update_;
    /// The last iteration number.
    uint64_t last_iter_;
    ///  The total number of iterations.
    uint64_t length_;
    /// The length of time, in milliseconds, to wait between updates.
    int interval_;
    /**
     * The minimum number of iterations that must pass before progress
     * reporting will be considered.
     */
    uint64_t min_iters_;
    /// The length of the last progress output message.
    uint64_t str_len_;
    /// Whether or not we have finished the job.
    bool finished_;
    /// Whether or not we should print an endline when done.
    bool endline_;
};
}
}
#endif
