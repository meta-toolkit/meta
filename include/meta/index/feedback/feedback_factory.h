//
// Created by Collin Gress on 11/6/16.
//

#ifndef META_FEEDBACK_FACTORY_H
#define META_FEEDBACK_FACTORY_H

#include "meta/index/feedback/feedback.h"
#include "meta/util/factory.h"

namespace cpptoml
{
class table;
}

namespace meta
{
namespace index
{

class feedback_factory
        : public util::factory<feedback_factory, feedback, const cpptoml::table&>
{
    friend base_factory;

    private:
        feedback_factory();

        template <class Feedback>
        void reg();
};

std::unique_ptr<feedback> make_feedback(const cpptoml::table&);

template <class Feedback>
std::unique_ptr<feedback> make_feedback(const cpptoml::table&)
{
    return make_unique<Feedback>();
}


class feedback_loader : public util::factory<feedback_loader, feedback, std::istream&>
{
    friend base_factory;

    private:
        feedback_loader();

        template <class Feedback>
        void reg();
};

std::unique_ptr<feedback> load_feedback(std::istream&);

template <class Feedback>
std::unique_ptr<feedback> load_feedback(std::istream& in)
{
    return make_unique<Feedback>(in);
}

template <class Feedback>
void register_feedback()
{
    feedback_factory::get().add(Feedback::id, make_feedback<Feedback>);
    feedback_loader::get().add(Feedback::id, load_feedback<Feedback>);
}

}
}


#endif //META_FEEDBACK_FACTORY_H
