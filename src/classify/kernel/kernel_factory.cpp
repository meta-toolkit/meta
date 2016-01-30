/**
 * @file kernel_factory.cpp
 * @author Chase Geigle
 */

#include "meta/classify/kernel/all.h"
#include "meta/classify/kernel/kernel_factory.h"
#include "meta/io/packed.h"

namespace meta
{
namespace classify
{
namespace kernel
{

template <class Kernel>
void kernel_factory::reg()
{
    add(Kernel::id, make_kernel<Kernel>);
}

kernel_factory::kernel_factory()
{
    // built-in kernels
    reg<polynomial>();
    reg<radial_basis>();
    reg<sigmoid>();
}

std::unique_ptr<kernel> make_kernel(const cpptoml::table& config)
{
    auto method = config.get_as<std::string>("method");
    if (!method)
        throw kernel_factory::exception{
            "method required to construct a kernel"};
    return kernel_factory::get().create(*method, config);
}

template <class Kernel>
void kernel_loader::reg()
{
    add(Kernel::id, load_kernel<Kernel>);
}

kernel_loader::kernel_loader()
{
    // built-in kernels
    reg<polynomial>();
    reg<radial_basis>();
    reg<sigmoid>();
}

std::unique_ptr<kernel> load_kernel(std::istream& in)
{
    std::string method;
    io::packed::read(in, method);
    return kernel_loader::get().create(method, in);
}
}
}
}
