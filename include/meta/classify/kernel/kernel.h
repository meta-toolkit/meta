/**
 * @file kernel.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_KERNEL_KERNEL_H_
#define META_CLASSIFY_KERNEL_KERNEL_H_

#include <ostream>
#include "meta/learn/dataset.h"

namespace meta
{
namespace classify
{
namespace kernel
{

using feature_vector = learn::feature_vector;

/**
 * Base class for kernels used in kernel-supporting classifiers.
 */
class kernel
{
  public:
    /**
     * Defaulted virtual destructor to support polymorphic deletion.
     */
    virtual ~kernel() = default;

    /**
     * Computes the value of \f$K(first, second)\f$.
     */
    virtual double operator()(const feature_vector& first,
                              const feature_vector& second) const = 0;

    /**
     * Saves the kernel to a stream. This should first save the kernel's
     * id, followed by any parameters needed for reconstruction.
     */
    virtual void save(std::ostream& out) const = 0;
};
}
}
}
#endif
