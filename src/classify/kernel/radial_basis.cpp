/**
 * @file radial_basis.cpp
 * @author Chase Geigle
 */

#include "meta/classify/kernel/radial_basis.h"
#include "meta/io/packed.h"

namespace meta
{
namespace classify
{
namespace kernel
{

const util::string_view radial_basis::id = "rbf";

radial_basis::radial_basis(double gamma) : gamma_{gamma}
{
    // nothing
}

radial_basis::radial_basis(std::istream& in)
{
    io::packed::read(in, gamma_);
}

void radial_basis::save(std::ostream& out) const
{
    io::packed::write(out, id);
    io::packed::write(out, gamma_);
}

double radial_basis::operator()(const feature_vector& first,
                                const feature_vector& second) const
{
    auto first_it = std::begin(first);
    auto first_end = std::end(first);

    auto second_it = std::begin(second);
    auto second_end = std::end(second);

    auto dist = 0.0;
    while (first_it != first_end && second_it != second_end)
    {
        if (first_it->first == second_it->first)
        {
            auto delta = first_it->second - second_it->second;
            dist += delta * delta;
            ++first_it;
            ++second_it;
        }
        else if (first_it->first < second_it->first)
        {
            dist += first_it->second * first_it->second;
            ++first_it;
        }
        else
        {
            dist += second_it->second * second_it->second;
            ++second_it;
        }
    }

    for (; first_it != first_end; ++first_it)
        dist += first_it->second * first_it->second;

    for (; second_it != second_end; ++second_it)
        dist += second_it->second * second_it->second;

    return std::exp(gamma_ * dist);
}

template <>
std::unique_ptr<kernel> make_kernel<radial_basis>(const cpptoml::table& config)
{
    auto gamma = config.get_as<double>("gamma");
    if (!gamma)
        throw kernel_factory::exception{
            "rbf kernel needs gamma in configuration"};
    return make_unique<radial_basis>(*gamma);
}
}
}
}
