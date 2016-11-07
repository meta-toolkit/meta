//
// Created by Collin Gress on 11/6/16.
//

#include "meta/index/feedback/rocchio.h"

namespace meta
{
namespace index
{
rocchio::rocchio(float a, float b, float c) : a_{a}, b_{b}, c_{c} {}

rocchio::rocchio(std::istream& in)
        : a_{io::packed::read<float>(in)},
          b_{io::packed::read<float>(in)},
          c_{io::packed::read<float>(in)}
{

}

corpus::document rocchio::apply_feedback(corpus::document q0,
                                         std::vector<corpus::document*> relevant,
                                         std::vector<corpus::document*> non_relevant,
                                         inverted_index &idx)
{
    auto q0_vect = q0.vector();
    corpus::document d;
    return d;
}

template <>
std::unique_ptr<feedback> make_feedback<rocchio>(const cpptoml::table& config)
{
    auto a = config.get_as<double>("a").value_or(rocchio::default_a);
    auto b = config.get_as<double>("b").value_or(rocchio::default_b);
    auto c = config.get_as<double>("c").value_or(rocchio::default_c);

    //TODO: validate these values

    return make_unique<rocchio>(a, b, c);
}



}
}