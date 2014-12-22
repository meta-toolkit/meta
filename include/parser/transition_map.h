/**
 * @file transition_map.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_TRANSITION_MAP_H_
#define META_PARSER_TRANSITION_MAP_H_

#include "parser/transition.h"
#include "util/sparse_vector.h"

namespace meta
{
namespace parser
{

class transition_map
{
  public:
    transition_map() = default;
    transition_map(const std::string& prefix);

    const transition& at(trans_id id) const;
    trans_id at(const transition& trans) const;

    transition& operator[](trans_id id);
    trans_id operator[](const transition& trans);

    void save(const std::string& prefix) const;

    uint64_t size() const;

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  private:
    util::sparse_vector<transition, trans_id> map_;
    std::vector<transition> transitions_;
};
}
}
#endif
