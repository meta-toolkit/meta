/**
 * @file phrase_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PHRASE_ANALYZER_H_
#define META_PHRASE_ANALYZER_H_

#include <unordered_set>
#include "analyzers/tree/tree_analyzer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 *
 */
class phrase_analyzer : public util::multilevel_clonable <analyzer,
                                tree_analyzer<phrase_analyzer>, phrase_analyzer>
{
  public:
    void tree_tokenize(corpus::document& doc, const parse_tree& tree);

    /// Identifier for this analyzer
    const static std::string id;

    std::vector<std::string> phrases();

  private:
    std::vector<std::string> phrases_;

    const static std::unordered_set<std::string> clause_tags_;
    const static std::unordered_set<std::string> phrase_tags_;
    const static std::unordered_set<std::string> pos_tags_;
};
}
}

#endif
