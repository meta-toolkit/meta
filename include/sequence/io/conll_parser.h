/**
 * @file conll_parser.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SEQUENCE_CONLL_PARSER_H_
#define META_SEQUENCE_CONLL_PARSER_H_

#include "sequence/sequence.h"

namespace meta
{
namespace sequence
{
namespace conll
{

class dataset
{
  public:
    /**
     * Reads a CONLL2000 formatted, part-of-speech and BIO tagged file and
     * returns a dataset consisting of the sequences parsed from it.
     *
     * For the sake of the analyzer, the sequences returned are POS tagged,
     * but the BIO tags are kept and may be requested on a per-instance
     * basis (for restoring the real labels prior to training/testing).
     *
     * @param filename The CONLL2000 formatted file
     * @return all of the sequences that were parsed from the given file
     */
    dataset(const std::string& filename);

    /**
     * @return the set of BIO-tagged sequences
     */
    std::vector<sequence>& sequences();

    /**
     * @param seq The sequence id
     * @param obs The observation index in that sequence
     * @return the BIO tag for the given index in the given sequence
     */
    const tag_t& tag(uint64_t seq, uint64_t obs) const;

  private:
    /// storage for the sequences
    std::vector<sequence> sequences_;
    /// storage for the BIO tags
    std::vector<std::vector<tag_t>> tags_;
};

}
}
}

#endif
