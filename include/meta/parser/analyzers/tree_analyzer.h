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

#include "meta/corpus/document.h"
#include "meta/analyzers/analyzer.h"
#include "meta/analyzers/analyzer_factory.h"
#include "meta/parser/analyzers/featurizers/tree_featurizer.h"
#include "meta/parser/sr_parser.h"
#include "meta/sequence/perceptron.h"
#include "meta/util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Base class tokenizing using parse tree features.
 *
 * Required config parameters:
 * ~~~toml
 * [[analyzers]]
 * method = "tree" # this analyzer
 * filter = [{type = "icu-tokenizer"}, {type = "ptb-normalizer"}] # example
 * features = ["skel", "subtree"] # example
 * tagger = "path"
 * parser = "path"
 * ~~~
 *
 * Optional config parameters: none.
 *
 * @see https://meta-toolkit.org/analyzers-filters-tutorial.html

 */
template <class T>
class tree_analyzer : public util::clonable<analyzer<T>, tree_analyzer<T>>
{
  public:
    using feature_map = typename analyzer<T>::feature_map;

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
     * Adds a tree featurizer to the list.
     */
    void add(std::unique_ptr<const tree_featurizer<T>> featurizer);

    /**
     * Identifier for this analyzer.
     */
    const static util::string_view id;

  private:
    using tree_featurizer_list
        = std::vector<std::unique_ptr<const tree_featurizer<T>>>;

    /**
     * Tokenizes a file into a document.
     * @param doc The document to store the tokenized information in
     */
    void tokenize(const corpus::document& doc, feature_map& counts) override;

    /**
     * A list of tree_featurizers to run on each parse tree.
     */
    std::shared_ptr<tree_featurizer_list> featurizers_;

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
 * Specialization of the traits class used by the factory method for
 * creating tree analyzers.
 */
template <class T>
struct analyzer_traits<tree_analyzer<T>>
{
    static std::unique_ptr<analyzer<T>> create(const cpptoml::table&,
                                               const cpptoml::table&);
};

// declare the valid instantiations of this analyzer
extern template class tree_analyzer<uint64_t>;
extern template class tree_analyzer<double>;

// declare the valid instantiations of this analyzer's traits class
extern template struct analyzer_traits<tree_analyzer<uint64_t>>;
extern template struct analyzer_traits<tree_analyzer<double>>;
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
