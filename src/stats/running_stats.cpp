/**
 * @file running_stats.cpp
 * @author Chase Geigle
 */

#include <cmath>
#include "meta/stats/running_stats.h"

namespace meta
{
namespace stats
{

running_stats::running_stats() : m_k_{0.0}, s_k_{0.0}, num_items_{0}
{
    // nothing
}

void running_stats::add(double value)
{
    ++num_items_;
    auto old_m_k = m_k_;

    m_k_ += (value - old_m_k) / num_items_;
    s_k_ += (value - m_k_) * (value - old_m_k);
}

double running_stats::mean() const
{
    return m_k_;
}

double running_stats::stddev() const
{
    return std::sqrt(variance());
}

double running_stats::variance() const
{
    return s_k_ / (num_items_ - 1);
}
}
}
