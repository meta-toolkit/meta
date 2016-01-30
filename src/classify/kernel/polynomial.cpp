/**
 * @file polynomial.cpp
 * @author Chase Geigle
 */

#include "meta/classify/kernel/polynomial.h"
#include "meta/io/packed.h"

namespace meta
{
namespace classify
{
namespace kernel
{

constexpr const uint8_t polynomial::default_power;
constexpr const double polynomial::default_c;
const util::string_view polynomial::id = "polynomial";

polynomial::polynomial(uint8_t power, double c) : power_{power}, c_{c}
{
    // nothing
}

polynomial::polynomial(std::istream& in)
{
    io::packed::read(in, power_);
    io::packed::read(in, c_);
}

double polynomial::operator()(const feature_vector& first,
                              const feature_vector& second) const
{
    return std::pow(util::dot_product(first, second) + c_, power_);
}

void polynomial::save(std::ostream& out) const
{
    io::packed::write(out, id);
    io::packed::write(out, power_);
    io::packed::write(out, c_);
}

template <>
std::unique_ptr<kernel> make_kernel<polynomial>(const cpptoml::table& config)
{
    auto power
        = config.get_as<int64_t>("power").value_or(polynomial::default_power);
    auto c = config.get_as<double>("c").value_or(polynomial::default_c);

    return make_unique<polynomial>(static_cast<uint8_t>(power), c);
}
}
}
}
