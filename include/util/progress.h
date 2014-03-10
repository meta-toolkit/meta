/**
 * @file progress.h
 * @author Chase Geigle
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
     * @param prefix the string to be printed right before the progress
     *  output
     * @param length the number of iterations
     * @param interval the length of time, in milliseconds, to wait
     * between updates. Default = 500ms.
     */
    progress(const std::string& prefix, uint64_t length,
             int interval = 500, uint64_t min_iters = 10);

    /**
     * Sets whether or not an endline should be printed at completion.
     */
    void print_endline(bool endline);

    /**
     * Destroys this progress reporter. It will call end() if it has not
     * already been called.
     */
    ~progress();

    /**
     * Updates the progress indicator.
     */
    void operator()(uint64_t iter);

    /**
     * Marks the progress indicator as having finished.
     */
    void end();

  private:
    std::string prefix_;
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point last_update_;
    uint64_t last_iter_;
    uint64_t length_;
    int interval_;
    uint64_t min_iters_;
    uint64_t str_len_;
    bool finished_;
    bool endline_;
};
}
}
#endif
