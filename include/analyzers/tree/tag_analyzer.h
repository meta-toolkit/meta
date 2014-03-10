/**
 * @file tag_analyzer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_TAG_ANALYZER_H_
#define META_TAG_ANALYZER_H_

#include "analyzers/tree/tree_analyzer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees by looking at labels of leaf and interior nodes.
 */
class tag_analyzer : public util::multilevel_clonable<analyzer,
    tree_analyzer<tag_analyzer>, tag_analyzer>
{
  public:
    /**
     * Counts occurrences of leaf and interior node labels.
     * @param doc The document to parse
     * @param tree The current parse_tree in the document
     */
    void tree_tokenize(corpus::document& doc, const parse_tree& tree);

    /**
     * Identifier for this analyzer.
     */
    const static std::string id;
};
}
}

#endif
