/**
 * @file embedding_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_EMBEDDINGS_EMBEDDING_ANALYZER_H_
#define META_EMBEDDINGS_EMBEDDING_ANALYZER_H_

#include "meta/analyzers/analyzer.h"
#include "meta/analyzers/analyzer_factory.h"
#include "meta/embeddings/word_embeddings.h"
#include "meta/util/clonable.h"
#include <string>

namespace meta
{
namespace analyzers
{

/**
 * Analyzes documents by averaging word embeddings for each token. This analyzer
 * should only be used with forward_index since it stores double feature values.
 *
 * Required config parameters:
 * ~~~toml
 * [[analyzers]]
 * method = "embedding" # this analyzer
 * filter = # use same filter type that embeddings were learned with
 * prefix = "path/to/embedding/model/"
 * ~~~
 */
class embedding_analyzer : public util::clonable<analyzer, embedding_analyzer>
{
  public:
    /**
     * Constructor.
     * @param stream The stream to read tokens from.
     */
    embedding_analyzer(const cpptoml::table& config,
                       std::unique_ptr<token_stream> stream);

    /**
     * Copy constructor.
     * @param other The other embedding_analyzer to copy from
     */
    embedding_analyzer(const embedding_analyzer& other);

    /// Identifier for this analyzer.
    const static util::string_view id;

  private:
    virtual void tokenize(const corpus::document& doc,
                          featurizer& counts) override;

    /// The token stream to be used for extracting tokens
    std::unique_ptr<token_stream> stream_;

    /// Learned word embeddings
    std::shared_ptr<embeddings::word_embeddings> embeddings_;

    /// Path to the embedding model files
    std::string prefix_;

    /// Storage for the aggregated word embeddings per document
    std::vector<double> features_;
};

/**
 * Specialization of the factory method for creating embedding_analyzers.
 */
template <>
std::unique_ptr<analyzer>
make_analyzer<embedding_analyzer>(const cpptoml::table&, const cpptoml::table&);
}

namespace embeddings
{
/**
 * Registers analyzers provided by the meta-embeddings library.
 */
void register_analyzers();
}
}
#endif
