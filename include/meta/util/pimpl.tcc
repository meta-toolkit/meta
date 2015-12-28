/**
 * @file pimpl.tcc
 * @author Chase Geigle
 * @see http://herbsutter.com/gotw/_101/
 */

#ifndef META_UTIL_PIMPL_TCC_
#define META_UTIL_PIMPL_TCC_

#include "meta/util/pimpl.h"

namespace meta
{
namespace util
{

template <class Impl>
pimpl<Impl>::pimpl()
    : impl_{make_unique<Impl>()}
{
    // nothing
}

template <class Impl>
pimpl<Impl>::pimpl(pimpl&&) = default;

template <class Impl>
pimpl<Impl>& pimpl<Impl>::operator=(pimpl&&) = default;

template <class Impl>
template <class... Args>
pimpl<Impl>::pimpl(Args&&... args)
    : impl_{make_unique<Impl>(std::forward<Args>(args)...)}
{
    // nothing
}

template <class Impl>
pimpl<Impl>::~pimpl() = default;

template <class Impl>
Impl* pimpl<Impl>::operator->()
{
    return impl_.get();
}

template <class Impl>
const Impl* pimpl<Impl>::operator->() const
{
    return impl_.get();
}

template <class Impl>
Impl& pimpl<Impl>::operator*()
{
    return *impl_;
}

template <class Impl>
const Impl& pimpl<Impl>::operator*() const
{
    return *impl_;
}
}
}
#endif
