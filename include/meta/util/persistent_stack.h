/**
 * @file persistent_stack.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_PERSISTENT_STACK_H_
#define META_UTIL_PERSISTENT_STACK_H_

#include <memory>

#include "meta/config.h"

namespace meta
{
namespace util
{

template <class T>
class persistent_stack
{
  public:
    persistent_stack();

    persistent_stack<T> push(T data) const;

    persistent_stack<T> pop() const;

    const T& peek() const;

    uint64_t size() const;

  private:
    struct node
    {
        node(T item, std::shared_ptr<node> previous);

        T data;
        std::shared_ptr<node> prev;
    };

    persistent_stack(std::shared_ptr<node> head, uint64_t size);

    std::shared_ptr<node> head_;
    uint64_t size_;
};

class persistent_stack_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#include "meta/util/persistent_stack.tcc"

#endif
