/**
 * @file corpus_factory.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CORPUS_CORPUS_FACTORY_
#define META_CORPUS_CORPUS_FACTORY_

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/corpus/corpus.h"
#include "meta/io/filesystem.h"
#include "meta/util/factory.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace corpus
{

/**
 * Factory that is responsible for creating corpus instances from
 * configuration files. Clients should use the register_corpus method
 * instead of this class directly to add their own corpus classes.
 *
 * The three parameters are:
 * - util::string_view prefix, the prefix for the datasets
 * - util::string_view dataset, the folder within the prefix
 * - const cpptoml::table& config, the corpus config object
 */
class corpus_factory
    : public util::factory<corpus_factory, corpus, util::string_view,
                           util::string_view, const cpptoml::table&>
{
    /// friend the base corpus factory
    friend base_factory;

    /**
     * Constructor.
     */
    corpus_factory();

    /**
     * Registers a corpus class.
     */
    template <class Corpus>
    void reg();
};

/**
 * Convenience method for creating a corpus using the factory. The
 * configuration object passed here should be the "global" configuration
 * (as in, the one that contains the "prefix", "dataset", and "corpus"
 * keys).
 */
std::unique_ptr<corpus> make_corpus(const cpptoml::table& config);

/**
 * Factory method for creating a corpus. This should be specialized if your
 * given corpus requires special construction behavior (e.g., reading
 * additional parameters).
 */
template <class Corpus>
std::unique_ptr<corpus> make_corpus(util::string_view prefix,
                                    util::string_view dataset,
                                    const cpptoml::table& config);
}
}
#endif
