/**
 * @file sequence_extractor.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include "meta/config.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/sequence/sequence.h"

namespace meta
{
namespace parser
{

/**
 * This is a visitor that converts a parse tree into a POS-tagged sequence.
 * This is currently used to extract test data from a treebank for the
 * parser.
 */
class sequence_extractor : public const_visitor<void>
{
  public:
    void operator()(const leaf_node&) override;
    void operator()(const internal_node&) override;

    /**
     * Extracts the sequence found. This moves the sequence out of the
     * extractor.
     *
     * @return the sequence that was extracted from the tree the visitor
     * was run on
     */
    sequence::sequence sequence();

  private:
    /// Storage for the partial sequence thus far
    sequence::sequence seq_;
};
}
}
