/**
 * @file kernel_factory.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CLASSIFY_KERNEL_KERNEL_FACTORY_H_
#define META_CLASSIFY_KERNEL_KERNEL_FACTORY_H_

#include <istream>
#include "meta/classify/kernel/kernel.h"
#include "meta/util/factory.h"

namespace meta
{
namespace classify
{
namespace kernel
{

/**
 * Factory that is responsible for creating kernels from configuration
 * tables. Clients should use the register_kernel method instead of this
 * class directly to add their own kernels.
 */
class kernel_factory
    : public util::factory<kernel_factory, kernel, const cpptoml::table&>
{
    friend base_factory;

  private:
    /**
     * Constructor for making the kernel_factory singleton.
     */
    kernel_factory();

    /**
     * Registers a kernel function; used internally.
     */
    template <class Kernel>
    void reg();
};

/**
 * Convenience method for making a kernel using the factory.
 */
std::unique_ptr<kernel> make_kernel(const cpptoml::table&);

/**
 * Factory method for creating a kernel. This should be specialized if your
 * given kernel requires special construction behavior (e.g., reading
 * parameters).
 */
template <class Kernel>
std::unique_ptr<kernel> make_kernel(const cpptoml::table&)
{
    return make_unique<Kernel>();
}

/**
 * Factory that is responsible for loading kernels from streams. Typically
 * this is done to load in a model that was trained with a kernel from its
 * model file.
 */
class kernel_loader : public util::factory<kernel_loader, kernel, std::istream&>
{
    friend base_factory;

  private:
    /**
     * Constructor for making the kernel_loader singleton.
     */
    kernel_loader();

    /**
     * Registers a kernel function; used internally.
     */
    template <class Kernel>
    void reg();
};

/**
 * Convenience method for loading a kernel using the factory.
 */
std::unique_ptr<kernel> load_kernel(std::istream& in);

/**
 * Factory method for loading a kernel. This should be specialized if your
 * given kernel requires special construction behavior. Otherwise, it is
 * assumed that the kernel has a constructor from a std::istream&.
 */
template <class Kernel>
std::unique_ptr<kernel> load_kernel(std::istream& in)
{
    return make_unique<Kernel>(in);
}

/**
 * Registration method for kernels. Clients should use this method to
 * register any new kernels they write.
 */
template <class Kernel>
void register_kernel()
{
    kernel_factory::get().add(Kernel::id, make_kernel<Kernel>);
    kernel_loader::get().add(Kernel::id, load_kernel<Kernel>);
}
}
}
}
#endif
