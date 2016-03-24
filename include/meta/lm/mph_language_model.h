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
#include "meta/lm/sentence.h"
#include "meta/lm/token_list.h"
#include "meta/util/array_view.h"

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

#if 0

    /**
     * Default move constructor.
     */
    mph_language_model(mph_language_model&&) = default;

    /**
     * @param sentence A sequence of tokens
     * @return the perplexity of this token sequence given the current language
     * model: \f$ \sqrt[n]{\prod_{i=1}^n\frac{1}{p(w_i|w_{i-n}\cdots w_{i-1})}}
     * \f$
     */
    float perplexity(const sentence& tokens) const;

    /**
     * @param sentence A sequence of tokens
     * @return the perplexity of this token sequence given the current language
     * model normalized by the length of the sequence
     */
    float perplexity_per_word(const sentence& tokens) const;

    /**
     * @param tokens A sequence of n tokens (one sentence)
     * @return the log probability of the likelihood of this sentence
     */
    float log_prob(const sentence& tokens) const;

  private:
    struct impl;
    std::unique_ptr<impl> impl_;
#endif
};
}
}
#endif
