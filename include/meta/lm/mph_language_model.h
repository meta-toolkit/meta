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
#include "meta/lm/ngram_map.h"
#include "meta/lm/sentence.h"
#include "meta/lm/token_list.h"
#include "meta/util/array_view.h"

namespace meta
{
namespace lm
{

struct lm_state
{
    std::vector<uint64_t> previous;

    void shrink()
    {
        std::copy(previous.begin() + 1, previous.end(), previous.begin());
        previous.pop_back();
    }
};

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

    float score(const lm_state& in_state, const std::string& token,
                lm_state& out_state) const;

    float score(const lm_state& in_state, uint64_t token,
                lm_state& out_state) const;

  private:
    float score(const lm_state& in_state, uint64_t token, prob_backoff<> pb,
                lm_state& out_state) const;

    struct impl;
    std::unique_ptr<impl> impl_;
};
}
}
#endif
