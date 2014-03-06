/**
 * @file loss_function.h
 * @author Chase Geigle
 */

#ifndef _META_CLASSIFY_LOSS_FUNCTION_H_
#define _META_CLASSIFY_LOSS_FUNCTION_H_

#include <string>

namespace meta
{
namespace classify
{
namespace loss
{

struct loss_function
{
    virtual double loss(double prediction, int expected) const = 0;
    virtual double derivative(double prediction, int expected) const  = 0;
};

}
}
}
#endif
