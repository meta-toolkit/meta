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

/**
 * An invertible map that maps transitions to ids.
 */
class transition_map
{
  public:
    /**
     * Default constructor.
     */
    transition_map() = default;

    /**
     * Loads a transition map from a prefix.
     * @param prefix The folder to load the map from
     */
    transition_map(const std::string& prefix);

    /**
     * @param id The id to look up
     * @return the transition corresponding to that id
     */
    const transition& at(trans_id id) const;

    /**
     * @param trans The transition to look up
     * @return the trans_id associated with that transition
     */
    trans_id at(const transition& trans) const;

    /**
     * Adds a transition to the map, if it doesn't already exist.
     *
     * @param trans The transition to look up
     * @param the trans_id associated with that transition.
     */
    trans_id operator[](const transition& trans);

    /**
     * Saves the map to a file stored in the folder indicated by prefix.
     * @param prefix The folder to save the map to
     */
    void save(const std::string& prefix) const;

    /**
     * @return the number of transitions in the map.
     */
    uint64_t size() const;

    /**
     * Exception thrown from interactions with the transition_map.
     */
    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  private:
    /**
     * The map from transition to id.
     */
    util::sparse_vector<transition, trans_id> map_;

    /**
     * The "map" from id to transition.
     */
    std::vector<transition> transitions_;
};
}
}
#endif
