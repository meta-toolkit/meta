/**
 * @file loss_function.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_LOSS_FUNCTION_H_
#define META_CLASSIFY_LOSS_FUNCTION_H_

#include <ostream>
#include <string>

namespace meta
{
namespace learn
{
namespace loss
{

/**
 * Base class for all loss functions that can be passed to the sgd
 * classifier. Each loss function must specify the value of the loss for a
 * given prediction vs. expected value, and also the derivative of the loss
 * function for a given prediction vs. expected value.
 *
 * @see http://dl.acm.org/citation.cfm?id=1015332
 */
struct loss_function
{
    /**
     * The loss incurred in assigning the given prediction value, given
     * the correct value of the prediction.
     *
     * @param prediction The prediction obtained from the model (dot
     * product)
     * @param expected The expected (as in, correct) value of the model's
     * prediction
     * @return the loss incurred
     */
    virtual double loss(double prediction, double expected) const = 0;

    /**
     * The derivative of the loss function given a predicted value and the
     * expected result of that prediction.
     *
     * @param prediction The prediction obtained from the model (dot
     * product)
     * @param expected The expected (as in, correct) value of the model's
     * prediction
     * @return the derivative of the loss function at that point
     */
    virtual double derivative(double prediction, double expected) const = 0;

    /**
     * Saves the loss function to a stream.
     * @param out The stream to write to
     */
    virtual void save(std::ostream& out) const = 0;
};
}
}
}
#endif
