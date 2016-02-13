/**
 * @file progress.cpp
 * @author Chase Geigle
 */

#include <cassert>
#include <cstdio>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "meta/logging/logger.h"
#include "meta/util/printing.h"
#include "meta/util/progress.h"

namespace meta
{
namespace printing
{

progress::progress(const std::string& prefix, uint64_t length, int interval)
    : prefix_len_{prefix.length()},
      start_{std::chrono::steady_clock::now()},
      iter_{0},
      length_{length},
      interval_{interval},
      endline_{true}
{
    output_.resize(80, ' ');
    assert(prefix_len_ < 80 - 20);
    std::copy(prefix.begin(), prefix.end(), output_.begin());
    output_[prefix_len_] = '[';

    thread_ = std::thread(std::bind(&progress::progress_thread, this));
}

void progress::print()
{
    using namespace std::chrono;
    auto iter = std::max(uint64_t{1}, iter_.load());
    auto tp = steady_clock::now();
    auto percent = static_cast<double>(iter) / length_;
    auto elapsed = duration_cast<milliseconds>(tp - start_).count();
    auto remain = (length_ - iter) * static_cast<double>(elapsed) / (iter);

    auto secs = static_cast<int>(remain / 1000);
    auto mins = secs / 60;
    auto hrs = mins / 60;

    std::ptrdiff_t max_len = 80 - static_cast<std::ptrdiff_t>(prefix_len_) - 20;
    if (hrs > 100)
        max_len -= 1;

    auto it = output_.begin() + static_cast<std::ptrdiff_t>(prefix_len_) + 1;
    auto barend = it + max_len;
    auto end = it + static_cast<std::ptrdiff_t>(max_len * percent);
    std::fill(it, end, '=');
    *end = '>';
    it = barend;
    *it++ = ']';
    *it++ = ' ';

    it += ::sprintf(&(*it), "%d%%", static_cast<int>(percent * 100));
    it += ::sprintf(&(*it), " ETA %02d:%02d:%02d", std::min(999, hrs),
                    mins % 60, secs % 60);

    LOG(progress) << '\r' << output_ << ENDLG;
}

void progress::progress_thread()
{
    while (iter_ != length_)
    {
        print();

        std::unique_lock<std::mutex> lock{mutex_};
        cond_var_.wait_for(lock, std::chrono::milliseconds(interval_));
    }
    print();
}

void progress::print_endline(bool endline)
{
    endline_ = endline;
}

void progress::operator()(uint64_t iter)
{
    iter_ = (iter < length_) ? iter : length_;
}

void progress::end()
{
    if (thread_.joinable())
    {
        iter_ = length_;
        cond_var_.notify_all();
        thread_.join();
        if (endline_)
            LOG(progress) << '\n' << ENDLG;
    }
}

void progress::clear() const
{
    LOG(progress) << '\r' << std::string(80, ' ') << '\r' << ENDLG;
}

progress::~progress()
{
    end();
}
}
}
