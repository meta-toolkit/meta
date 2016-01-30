/**
 * @file sigmoid.cpp
 * @author Chase Geigle
 */

#include "meta/classify/kernel/sigmoid.h"
#include "meta/io/packed.h"

namespace meta
{
namespace classify
{
namespace kernel
{

const util::string_view sigmoid::id = "sigmoid";

sigmoid::sigmoid(double alpha, double c) : alpha_{alpha}, c_{c}
{
    // nothing
}

sigmoid::sigmoid(std::istream& in)
{
    io::packed::read(in, alpha_);
    io::packed::read(in, c_);
}

double sigmoid::operator()(const feature_vector& first,
                           const feature_vector& second) const
{
    return std::tanh(alpha_ * util::dot_product(first, second) + c_);
}

void sigmoid::save(std::ostream& out) const
{
    io::packed::write(out, id);
    io::packed::write(out, alpha_);
    io::packed::write(out, c_);
}

template <>
std::unique_ptr<kernel> make_kernel<sigmoid>(const cpptoml::table& config)
{
    auto alpha = config.get_as<double>("alpha");
    if (!alpha)
        throw kernel_factory::exception{
            "sigmoid kernel needs alpha in configuration"};
    auto c = config.get_as<double>("c");
    if (!c)
        throw kernel_factory::exception{
            "sigmoid kernel needs c in configuration"};
    return make_unique<sigmoid>(*alpha, *c);
}
}
}
}
