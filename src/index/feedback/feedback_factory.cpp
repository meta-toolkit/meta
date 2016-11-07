//
// Created by Collin Gress on 11/6/16.
//

#include "cpptoml.h"
#include "meta/index/feedback/all.h"

namespace meta
{
namespace index
{

template<class Feedback>
void feedback_factory::reg() {
    add(Feedback::id, make_feedback<Feedback>);
}

feedback_factory::feedback_factory()
{
    reg<rocchio>();
}

std::unique_ptr<feedback> make_feedback(const cpptoml::table& config)
{
    auto function = config.get_as<std::string>("method");
    if (!function)
    {
        // TODO: implement exception
    }
    return feedback_factory::get().create(*function, config);
}

template <class Feedback>
void feedback_loader::reg()
{
    add(Feedback::id, load_feedback<Feedback>);
}

feedback_loader::feedback_loader()
{
    reg<rocchio>();
}

std::unique_ptr<feedback> load_feedback(std::istream& in)
{
    std::string method;
    io::packed::read(in, method);
    return feedback_loader::get().create(method, in);
}

}
}