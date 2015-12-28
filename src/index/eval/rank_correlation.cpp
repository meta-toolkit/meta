/**
 * @file rank_correlation.cpp
 * @author Chase Geigle
 */

#include "meta/index/eval/rank_correlation.h"

namespace meta
{
namespace index
{

namespace
{
struct rank_pair
{
    double xi;
    double yi;

    double xj;
    double yj;

    bool concordant() const
    {
        return (xi < xj && yi < yj) || (xi > xj && yi > yj);
    }

    bool discordant() const
    {
        return (xi < xj && yi > yj) || (xi > xj && yi < yj);
    }

    bool tied() const
    {
        return tied_x() && tied_y();
    }

    bool tied_x() const
    {
        return xi == xj;
    }

    bool tied_y() const
    {
        return yi == yj;
    }
};
}

rank_correlation::rank_correlation(const std::vector<double>& x,
                                   const std::vector<double>& y)
    : num_concordant_{0},
      num_discordant_{0},
      num_ties_x_{0},
      num_ties_y_{0},
      n_{x.size()}
{
    if (x.size() != y.size())
        throw exception{"Ranked lists must have the same size"};

    for (std::size_t i = 0; i < x.size(); ++i)
    {
        for (std::size_t j = i + 1; j < x.size(); ++j)
        {
            // determine whether (x[i], y[i]) and (x[j], y[j]) are
            // discordant, concordant, tied in both x and y, tied in x, or
            // just tied in y
            rank_pair pr{x[i], y[i], x[j], y[j]};
            if (pr.concordant())
            {
                ++num_concordant_;
            }
            else if (pr.discordant())
            {
                ++num_discordant_;
            }
            else if (pr.tied_x() && !pr.tied_y())
            {
                ++num_ties_x_;
            }
            else if (pr.tied_y() && !pr.tied_x())
            {
                ++num_ties_y_;
            }
        }
    }
}

double rank_correlation::gamma() const
{
    return (nc() - nd()) / (nc() + nd());
}

double rank_correlation::tau_a() const
{
    // (nc - nd) / (n(n+1)/2)
    return 2.0 * (nc() - nd()) / (n_ * (n_ - 1));
}

double rank_correlation::tau_b() const
{
    return (nc() - nd()) / std::sqrt((nc() + nd() + num_ties_x_)
                                     * (nc() + nd() + num_ties_y_));
}

double rank_correlation::ndpm() const
{
    auto cu = nc() + nd() + num_ties_x_;
    auto cu0 = num_ties_x_;
    return (2 * nd() + cu0) / (2 * cu);
}
}
}
