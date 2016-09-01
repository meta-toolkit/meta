/**
 * @file mph_language_model.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LM_MPH_LANGUAGE_MODEL_H_
#define META_LM_MPH_LANGUAGE_MODEL_H_

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/lm/lm_state.h"
#include "meta/lm/ngram_map.h"
#include "meta/lm/sentence.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace lm
{

/**
 * An ngram language model class based on a collection of minimal perfect
 * hash functions and dense value arrays. The modle generates minimal
 * perfect hash functions for each order in linear time, facilitating
 * \f$O(1)\f$ lookup time for any ngram of any order whilst using very
 * little space per ngram. Currently, estimation of the language model is
 * not implemented. We recommend using KenLM to generate a .arpa file from
 * a text corpus that has been (optionally) preprocessed by MeTA.
 *
 * @see http://www.speech.sri.com/projects/srilm/manpages/ngram-format.5.html
 * @see https://kheafield.com/code/kenlm/
 *
 * Required config parameters:
 * ~~~toml
 * [language-model]
 * binary-file-prefix = "path-to-binary-files"
 * # only this key is needed if the LM is already binarized
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * [language-model]
 * arpa-file = "path-to-arpa-file" # if no binary files have yet been created
 * ~~~
 */
class mph_language_model
{
  public:
    /**
    * Creates an N-gram language model based on the corpus specified in the
    * config file.
    */
    mph_language_model(const cpptoml::table& config);

    ~mph_language_model();

    /**
     * Determines to word index in the unigram table for this term.
     * @param token The unigram to look up
     */
    term_id index(util::string_view token) const;

    /**
     * @return the word index of the <unk> token.
     */
    term_id unk() const;

    /**
     * Returns the score according to the language model for generating
     * the next token given the current state in_state. The context needed
     * for scoring the next word is written to out_state.
     *
     * @param in_state The context, which is either just <s> or was
     * filled for you by a previous call to score()
     * @param token The next token to score (as a string)
     * @param out_state Storage to write the state for the next query to
     *
     * @return \f$p(w_n \mid w_1, \ldots, w_{n-1})\f$
     */
    float score(const lm_state& in_state, util::string_view token,
                lm_state& out_state) const;

    /**
     * Returns the score according to the language model for generating
     * the next token given the current state in_state. The context needed
     * for scoring the next word is written to out_state.
     *
     * @param in_state The context, which is either just <s> or was
     * filled for you by a previous call to score()
     * @param token The next token to score (as a word index)
     * @param out_state Storage to write the state for the next query to
     *
     * @return \f$p(w_n \mid w_1, \ldots, w_{n-1})\f$
     */
    float score(const lm_state& in_state, term_id token,
                lm_state& out_state) const;

  private:
    float score(const lm_state& in_state, term_id token, prob_backoff<> pb,
                lm_state& out_state) const;

    struct impl;
    std::unique_ptr<impl> impl_;
};
}
}
#endif
