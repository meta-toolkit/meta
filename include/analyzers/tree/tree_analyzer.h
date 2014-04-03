/**
 * @file tree_analyzer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_TREE_ANALYZER_H_
#define META_TREE_ANALYZER_H_

#include "corpus/document.h"
#include "analyzers/analyzer.h"
#include "analyzers/tree/parse_tree.h"

namespace meta
{
namespace analyzers
{

/**
 * Tokenizes parse trees with various methods.
 */
template <class DerivedTokenizer>
class tree_analyzer : public analyzer
{
  public:
    /**
     * Tokenizes a file into a document.
     * @param doc The document to store the tokenized information in
     */
    void tokenize(corpus::document& doc) override
    {
        std::vector<parse_tree> trees
            = parse_tree::get_trees(doc.path() + ".tree");
        for (auto& tree : trees)
            derived().tree_tokenize(doc, tree);
    }

  private:
    /**
     * Convenience method to obtain this analyzer as its derived class.
     * @return the derived version of the analyzer
     */
    DerivedTokenizer& derived()
    {
        return static_cast<DerivedTokenizer&>(*this);
    }
};
}
}

#endif
