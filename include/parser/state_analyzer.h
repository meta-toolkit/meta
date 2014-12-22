/**
 * @file state_analyzer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_STATE_ANALYZER_H_
#define META_PARSER_STATE_ANALYZER_H_

#include "parser/sr_parser.h"

namespace meta
{
namespace parser
{

class sr_parser::state_analyzer
{
  public:
    feature_vector featurize(const state& state) const;

  private:
    void unigram_featurize(const state& state, feature_vector& feats) const;

    void bigram_featurize(const state& state, feature_vector& feats) const;

    void trigram_featurize(const state& state, feature_vector& feats) const;

    void children_featurize(const state& state, feature_vector& feats) const;

    void unigram_stack_feats(const node* n, std::string prefix,
                             feature_vector& feats) const;

    void child_feats(const node* n, std::string prefix, feature_vector& feats,
                     bool doubs) const;
};
}
}

#endif
