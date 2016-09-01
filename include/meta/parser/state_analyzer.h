/**
 * @file state_analyzer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_STATE_ANALYZER_H_
#define META_PARSER_STATE_ANALYZER_H_

#include "meta/config.h"
#include "meta/parser/sr_parser.h"
#include "meta/parser/state.h"

namespace meta
{
namespace parser
{

/**
 * Analyzer responsible for converting a parser state to a
 * feature_vector.
 */
class sr_parser::state_analyzer
{
  public:
    /**
     * Maps a state to its feature vector representation.
     */
    feature_vector featurize(const state& state) const;

  private:
    /**
     * Adds unigram features.
     * @param state The current parser state
     * @param feats The feature vector to put features in
     */
    void unigram_featurize(const state& state, feature_vector& feats) const;

    /**
     * Adds bigram features.
     * @param state The current parser state
     * @param feats The feature vector to put features in
     */
    void bigram_featurize(const state& state, feature_vector& feats) const;

    /**
     * Adds trigram features.
     * @param state The current parser state
     * @param feats The feature vector to put features in
     */
    void trigram_featurize(const state& state, feature_vector& feats) const;

    /**
     * Adds children features.
     * @param state The current parser state
     * @param feats The feature vector to put features in
     */
    void children_featurize(const state& state, feature_vector& feats) const;

    /**
     * Adds dependent features.
     * @param state The current parser state
     * @param feats The feature vector to put features in
     */
    void dependents_featurize(const state& state, feature_vector& feats) const;

    /**
     * Adds unigram features from the parser stack.
     * @param n The node from the stack
     * @param prefix The feature name prefix
     * @param feats The feature vector to put features in
     */
    void unigram_stack_feats(const node* n, std::string prefix,
                             feature_vector& feats) const;

    /**
     * Adds bigram features to the feature vector.
     * @param n1 The first node
     * @param name1 The feature name prefix for the first node
     * @param n2 The second node
     * @param name2 The feature name prefix of the second node
     * @param feats The feature vector put features in
     */
    void bigram_features(const node* n1, std::string name1, const node* n2,
                         std::string name2, feature_vector& feats) const;

    /**
     * Adds child features to the feature vector.
     * @param n The node to add child features for
     * @param prefix The feature name prefix
     * @param feats The feature vector to put features in
     * @param doubs Whether or not to add features for children two steps
     * down
     */
    void child_feats(const node* n, std::string prefix, feature_vector& feats,
                     bool doubs) const;
};
}
}

#endif
