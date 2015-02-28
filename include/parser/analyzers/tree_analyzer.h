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
#include "analyzers/analyzer_factory.h"
#include "parser/analyzers/featurizers/tree_featurizer.h"
#include "parser/sr_parser.h"
#include "sequence/perceptron.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Base class tokenizing using parse tree features.
 */
class tree_analyzer : public util::clonable<analyzer, tree_analyzer>
{
  public:
    /**
     * Creates a tree analyzer
     */
    tree_analyzer(std::unique_ptr<token_stream> stream,
                  const std::string& tagger_prefix,
                  const std::string& parser_prefix);

    /**
     * Copy constructor.
     * @param other The other tree_analyzer to copy from.
     */
    tree_analyzer(const tree_analyzer& other);

    /**
     * Tokenizes a file into a document.
     * @param doc The document to store the tokenized information in
     */
    void tokenize(corpus::document& doc) override;

    /**
     * Adds a tree featurizer to the list.
     */
    void add(std::unique_ptr<const tree_featurizer> featurizer);

    /**
     * Identifier for this analyzer.
     */
    const static std::string id;

  private:
    /**
     * A list of tree_featurizers to run on each parse tree.
     */
    std::shared_ptr<std::vector<std::unique_ptr<const tree_featurizer>>>
        featurizers_;

    /**
     * The token stream for extracting tokens.
     */
    std::unique_ptr<token_stream> stream_;

    /**
     * The tagger used for tagging individual sentences. This will be
     * shared among all copies of this analyzer (so there will be only one
     * copy of the tagger's model across all of the threads used during
     * tokenization).
     */
    std::shared_ptr<const sequence::perceptron> tagger_;

    /**
     * The parser to parse individual sentences. This will be shared among
     * all copies of tis analyzer (so there will be one copy of the
     * parser's model across all of the threads used during tokenization).
     */
    std::shared_ptr<const parser::sr_parser> parser_;
};

/**
 * Specialization of the factory method for creating tree analyzers.
 */
template <>
std::unique_ptr<analyzer> make_analyzer<tree_analyzer>(const cpptoml::table&,
                                                       const cpptoml::table&);
}

namespace parser
{
/**
 * Register analyzers provided by the meta-parser-analyzers library.
 */
void register_analyzers();
}
}
#endif
