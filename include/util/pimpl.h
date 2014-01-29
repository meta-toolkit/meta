/**
 * @file pimpl.h
 * @author Chase Geigle
 * @see http://herbsutter.com/gotw/_101/
 */

#ifndef _META_UTIL_PIMPL_H_
#define _META_UTIL_PIMPL_H_

#include <memory>
#include "util/shim.h"

namespace meta
{
namespace util
{

template <class Impl>
class pimpl
{
  public:
    pimpl();
    pimpl(pimpl&&);
    pimpl& operator=(pimpl&&);

    template <class... Args>
    pimpl(Args&&... args);

    ~pimpl();

    Impl* operator->();
    const Impl* operator->() const;
    Impl& operator*();
    const Impl& operator*() const;
  private:
    std::unique_ptr<Impl> impl_;
};
}
}

#endif
