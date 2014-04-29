/**
 * @file progress.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <iomanip>
#include <sstream>
#include "logging/logger.h"
#include "util/printing.h"
#include "util/progress.h"

namespace meta
{
namespace printing
{

namespace
{
std::string eta_str(int milliseconds)
{
    int hrs = milliseconds / 1000 / 60 / 60;
    int ms = milliseconds % (1000 * 60 * 60);

    int min = ms / 1000 / 60;
    ms = ms % (1000 * 60);

    int sec = ms / 1000;

    std::stringstream ss;
    ss << "ETA " << std::setfill('0') << std::setw(2) << hrs << ':'
       << std::setfill('0') << std::setw(2) << min << ':' << std::setfill('0')
       << std::setw(2) << sec;
    return ss.str();
}
}

progress::progress(const std::string& prefix, uint64_t length, int interval,
                   uint64_t min_iters)
    : prefix_{prefix},
      start_{std::chrono::steady_clock::now()},
      last_update_{start_},
      last_iter_{0},
      length_{length},
      interval_{interval},
      min_iters_{min_iters},
      str_len_{0},
      finished_{false},
      endline_{true}
{
    // nothing
}

void progress::print_endline(bool endline)
{
    endline_ = endline;
}

void progress::operator()(uint64_t iter)
{
    using namespace std::chrono;
    if (iter - last_iter_ < min_iters_ && iter != length_)
        return;

    auto tp = steady_clock::now();
    if (duration_cast<milliseconds>(tp - last_update_).count() < interval_
        && iter != length_)
        return;

    last_update_ = tp;
    last_iter_ = iter;

    auto percent = static_cast<double>(iter) / length_;
    auto elapsed = duration_cast<milliseconds>(tp - start_).count();
    int remain = static_cast<double>(elapsed) / iter * (length_ - iter);

    std::stringstream ss;
    ss << prefix_;

    auto eta = eta_str(remain);
    // 4 comes from +2 for the [], +5 for the %, +2 for space
    auto remaining_width = std::max
        <int>(0, 80 - prefix_.length() - eta.length() - 9);
    if (remaining_width > 15)
    {
        auto filled = static_cast<int>(remaining_width * percent);
        auto empty = remaining_width - filled - 1;

        ss << '[' << std::string(filled, '=');
        if (filled != remaining_width)
        {
            ss << '>' << std::string(std::max<int>(0, empty), ' ');
        }
        ss << ']';
    }

    ss << ' ' << static_cast<int>(percent * 100) << "% " << eta;

    std::string rem(std::max<int>(0, str_len_ - ss.tellp()), ' ');
    str_len_ = ss.tellp();
    ss << rem;

    LOG(progress) << '\r' << ss.str() << ENDLG;
}

void progress::end()
{
    finished_ = true;
    if (last_iter_ != length_)
        (*this)(length_);
    if (endline_)
        LOG(progress) << '\n' << ENDLG;
}

void progress::clear() const
{
    LOG(progress) << '\r' << std::string(80, ' ') << '\r' << ENDLG;
}

progress::~progress()
{
    if (!finished_)
        end();
}
}
}
