/**
 * @file lm_state.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LM_LM_STATE_H_
#define META_LM_LM_STATE_H_

#include <algorithm>
#include <cstdint>
#include <vector>

#include "meta/config.h"
#include "meta/meta.h"

namespace meta
{
namespace lm
{
struct lm_state
{
    std::vector<term_id> previous;

    void shrink()
    {
        std::copy(previous.begin() + 1, previous.end(), previous.begin());
        previous.pop_back();
    }
};
}
}
#endif
